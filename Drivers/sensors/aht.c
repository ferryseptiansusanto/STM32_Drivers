/*
 * AHT.c
 *
 *  Created on: 19 Jun 2026
 *      Author: ferry
 */


/*
 * AHT.c
 *
 * Created on: 19 Jun 2026
 * Author: ferry
 */

#include "AHT.h"
#include "delay.h"   // Memakai GetTick() dan DelayMs() Anda
#include "FreeRTOS.h"
#include "task.h"

AHT_Status AHT_Init(AHT_Device *dev, I2C_HandleTypeDef *hi2c) {
    dev->hi2c = hi2c;
    dev->last_result = false;

    // Set waktu awal agar bisa langsung dibaca pertama kali tanpa interupsi cache
    dev->last_read_time = GetTick() - pdMS_TO_TICKS(AHT_MIN_INTERVAL);

    // 1. Cek apakah device merespon di bus I2C
    if (HAL_I2C_IsDeviceReady(dev->hi2c, AHT_I2C_ADDRESS, 3, 100) != HAL_OK) {
        return AHT_ERROR;
    }

    // 2. Soft Reset Sensor untuk memastikan kondisi awal bersih
    uint8_t rst_cmd = AHT_CMD_SOFTRESET;
    HAL_I2C_Master_Transmit(dev->hi2c, AHT_I2C_ADDRESS, &rst_cmd, 1, 100);
    DelayMs(20); // Tunggu sensor bangun setelah reset

    // 3. Ambil byte status awal untuk cek kalibrasi
    uint8_t status = 0;
    HAL_I2C_Master_Receive(dev->hi2c, AHT_I2C_ADDRESS, &status, 1, 100);

    // 4. Logika Adafruit: Jika belum terkalibrasi (Bit 3 == 0), kirim perintah inisialisasi
    if ((status & AHT_STATUS_CALBOOT) == 0) {
        uint8_t init_cmd[] = {AHT_CMD_INITIALIZE, 0x08, 0x00};
        if (HAL_I2C_Master_Transmit(dev->hi2c, AHT_I2C_ADDRESS, init_cmd, 3, 100) != HAL_OK) {
            return AHT_ERROR;
        }
        DelayMs(20); // Beri waktu sensor memproses kalibrasi
    }

    return AHT_OK;
}

AHT_Status AHT_Read(AHT_Device *dev, AHT_Data *output_data, bool force) {
    // 1. Proteksi Cache Interval menggunakan Tick FreeRTOS Anda
    uint32_t current_time = GetTick();
    if (!force && ((current_time - dev->last_read_time) < pdMS_TO_TICKS(AHT_MIN_INTERVAL))) {
        if (dev->last_result) {
            *output_data = dev->cached_data;
            return AHT_OK;
        }
        return AHT_TIMEOUT;
    }
    dev->last_read_time = current_time;

    // Reset buffer data mentah
    for (int i = 0; i < 6; i++) dev->data[i] = 0;

    // 2. Kirim Perintah Trigger Pengukuran (0xAC, 0x33, 0x00)
    uint8_t trigger_cmd[] = {AHT_CMD_TRIGGER, 0x33, 0x00};
    if (HAL_I2C_Master_Transmit(dev->hi2c, AHT_I2C_ADDRESS, trigger_cmd, 3, 100) != HAL_OK) {
        dev->last_result = false;
        return AHT_ERROR;
    }

    // 3. Jeda Pengukuran Non-blocking (RAMAH RTOS!)
    // AHT butuh ~75ms hingga 80ms untuk konversi data internal.
    // Selama DelayMs ini, task lain di FreeRTOS dapat berjalan bebas.
    DelayMs(80);

    // 4. Baca 6 Byte Data dari Sensor (1 Byte Status + 5 Byte Data Suhu & Kelembaban)
    if (HAL_I2C_Master_Receive(dev->hi2c, AHT_I2C_ADDRESS, dev->data, 6, 100) != HAL_OK) {
        dev->last_result = false;
        return AHT_ERROR;
    }

    // 5. Validasi Bit Status Busy (Bit 7 dari byte ke-0)
    if ((dev->data[0] & AHT_STATUS_BUSY) != 0) {
        dev->last_result = false;
        return AHT_TIMEOUT; // Kembalikan timeout jika sensor masih sibuk
    }

    // 6. Ekstraksi Data Mentah 20-Bit Sesuai Aturan Datasheet & Library Adafruit
    uint32_t raw_humidity = (((uint32_t)dev->data[1] << 12) |
                             ((uint32_t)dev->data[2] << 4)  |
                             ((dev->data[3] & 0xF0) >> 4));

    uint32_t raw_temperature = ((((uint32_t)dev->data[3] & 0x0F) << 16) |
                                 ((uint32_t)dev->data[4] << 8)  |
                                 dev->data[5]);

    // 7. Konversi Matematika Float
    dev->cached_data.humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    dev->cached_data.temperature = ((float)raw_temperature * 200.0f / 1048576.0f) - 50.0f;

    dev->last_result = true;
    *output_data = dev->cached_data;

    return AHT_OK;
}
