/*
 * AHT.c
 *
 * Created on: 19 Jun 2026
 * Author: ferry
 */

#include "AHT.h"
#include "delay.h" // Menyediakan GetTick() dan DelayMs() Anda

static AHT_Status Convert_I2C_Status(I2C_Status status) {
    switch (status) {
        case I2C_OK:      return AHT_OK;
        case I2C_TIMEOUT: return AHT_TIMEOUT;
        default:          return AHT_ERROR;
    }
}

AHT_Status AHT_Init(AHT_Device *dev, I2C_Context *ctx) {
    // Hubungkan context wrapper ke dalam objek device secara otomatis
    dev->i2c_ctx = ctx;
    dev->mode = I2C_MODE_IT; // Set mode default (Bisa diubah ke I2C_MODE_DMA jika diinginkan)
    dev->last_result = false;

    // Set waktu awal menggunakan wrapper GetTick() Anda
    dev->last_read_time = GetTick() - pdMS_TO_TICKS(AHT_MIN_INTERVAL_MS);

    // 1. Ambil Mutex I2C secara aman sebelum mengecek hardware via HAL standar
    if (xSemaphoreTake(dev->i2c_ctx->mutex, portMAX_DELAY) == pdTRUE) {
        if (HAL_I2C_IsDeviceReady(dev->i2c_ctx->hi2c, AHT_I2C_ADDRESS, 3, 100) != HAL_OK) {
            xSemaphoreGive(dev->i2c_ctx->mutex);
            return AHT_ERROR;
        }
        xSemaphoreGive(dev->i2c_ctx->mutex);
    } else {
        return AHT_ERROR;
    }

    // 2. Soft Reset Sensor menggunakan Transmit Wrapper Anda
    uint8_t rst_cmd = AHT_CMD_SOFTRESET;
    if (I2C_Transmit(dev->i2c_ctx, AHT_I2C_ADDRESS, dev->mode, &rst_cmd, 1, 100) != I2C_OK) {
        return AHT_ERROR;
    }
    DelayMs(20);

    // 3. Ambil byte status awal via Receive Wrapper Anda
    uint8_t status_byte = 0;
    if (I2C_Receive(dev->i2c_ctx, AHT_I2C_ADDRESS, dev->mode, &status_byte, 1, 100) != I2C_OK) {
        return AHT_ERROR;
    }

    // 4. Logika inisialisasi kalibrasi jika Bit 3 == 0 (Belum Terkalibrasi)
    if ((status_byte & AHT_STATUS_CALBOOT) == 0) {
        uint8_t init_cmd[] = {AHT_CMD_INITIALIZE, 0x08, 0x00};
        if (I2C_Transmit(dev->i2c_ctx, AHT_I2C_ADDRESS, dev->mode, init_cmd, 3, 100) != I2C_OK) {
            return AHT_ERROR;
        }
        DelayMs(20);
    }

    return AHT_OK;
}

AHT_Status AHT_Read(AHT_Device *dev, AHT_Data *output_data, bool force) {
    // 1. Proteksi Cache Interval menggunakan GetTick() Anda
    uint32_t current_time = GetTick();
    if (!force && ((current_time - dev->last_read_time) < pdMS_TO_TICKS(AHT_MIN_INTERVAL_MS))) {
        if (dev->last_result) {
            *output_data = dev->cached_data;
            return AHT_OK;
        }
        return AHT_TIMEOUT;
    }
    dev->last_read_time = current_time;

    // Bersihkan buffer data mentah array 6 byte
    for (int i = 0; i < 6; i++) dev->raw_buffer[i] = 0;

    // 2. Kirim Perintah Trigger Pengukuran (0xAC, 0x33, 0x00) via Wrapper
    uint8_t trigger_cmd[] = {AHT_CMD_TRIGGER, 0x33, 0x00};
    I2C_Status tx_status = I2C_Transmit(dev->i2c_ctx, AHT_I2C_ADDRESS, dev->mode, trigger_cmd, 3, 100);
    if (tx_status != I2C_OK) {
        dev->last_result = false;
        return Convert_I2C_Status(tx_status);
    }

    // 3. Jeda Pengukuran Non-blocking (RAMAH RTOS!)
    DelayMs(80);

    // 4. Baca 6 Byte Data dari Sensor via Wrapper
    I2C_Status rx_status = I2C_Receive(dev->i2c_ctx, AHT_I2C_ADDRESS, dev->mode, dev->raw_buffer, 6, 100);
    if (rx_status != I2C_OK) {
        dev->last_result = false;
        return Convert_I2C_Status(rx_status);
    }

    // 5. Validasi Bit Status Busy (Bit 7 dari byte ke-0 harus berstatus 0)
    if ((dev->raw_buffer[0] & AHT_STATUS_BUSY) != 0) {
        dev->last_result = false;
        return AHT_TIMEOUT;
    }

    // 6. Ekstraksi Data Mentah 20-Bit Sesuai Aturan Datasheet (Gunakan indeks buffer yang benar)
    uint32_t raw_humidity = (((uint32_t)dev->raw_buffer[1] << 12) |
                             ((uint32_t)dev->raw_buffer[2] << 4)  |
                             ((dev->raw_buffer[3] & 0xF0) >> 4));

    uint32_t raw_temperature = ((((uint32_t)dev->raw_buffer[3] & 0x0F) << 16) |
                                 ((uint32_t)dev->raw_buffer[4] << 8)  |
                                 dev->raw_buffer[5]);

    // 7. Konversions Matematika Float
    dev->cached_data.humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    dev->cached_data.temperature = ((float)raw_temperature * 200.0f / 1048576.0f) - 50.0f;

    dev->last_result = true;
    *output_data = dev->cached_data;

    return AHT_OK;
}
