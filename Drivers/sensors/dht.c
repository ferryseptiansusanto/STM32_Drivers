#include "dht.h"
#include "delay.h"   // Memakai GetTick(), DelayMs(), dan DelayUs() Anda
#include "FreeRTOS.h"
#include "task.h"

static void DHT_SetPinOutput(DHT_Device *dev) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dev->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(dev->port, &GPIO_InitStruct);
}

static void DHT_SetPinInput(DHT_Device *dev) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dev->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // Sesuai INPUT_PULLUP Adafruit
    HAL_GPIO_Init(dev->port, &GPIO_InitStruct);
}

// Mengukur durasi pulsa dengan memanfaatkan DelayUs(1) milik Anda (Pengganti expectPulse)
static uint32_t DHT_ExpectPulse(DHT_Device *dev, GPIO_PinState level) {
    uint32_t count = 0;
    uint32_t pin_mask = dev->pin;

    // Melakukan loop pembacaan register pin secara langsung demi kecepatan maksimal
    while ((dev->port->IDR & pin_mask) == (level ? pin_mask : 0)) {
        DelayUs(1);
        if (count++ >= 120) {
            return UINT32_MAX; // Timeout jika pulsa melebihi batas wajar (~120us)
        }
    }
    return count;
}

void DHT_Init(DHT_Device *dev, GPIO_TypeDef *port, uint16_t pin, DHT_Type type) {
    dev->port = port;
    dev->pin = pin;
    dev->type = type;
    dev->last_result = false;

    // Set waktu awal agar sensor bisa langsung dibaca pertama kali tanpa interupsi cache
    dev->last_read_time = GetTick() - pdMS_TO_TICKS(MIN_INTERVAL);

    DHT_SetPinInput(dev);
}

DHT_Status DHT_Read(DHT_Device *dev, DHT_Data *output_data, bool force) {
    // 1. Proteksi Cache 2 Detik ala Adafruit menggunakan Tick FreeRTOS Anda
    uint32_t current_time = GetTick();
    if (!force && ((current_time - dev->last_read_time) < pdMS_TO_TICKS(MIN_INTERVAL))) {
        if (dev->last_result) {
            *output_data = dev->cached_data;
            return DHT_OK;
        }
        return DHT_TIMEOUT;
    }
    dev->last_read_time = current_time;

    // Reset isi buffer data mentah
    dev->data[0] = dev->data[1] = dev->data[2] = dev->data[3] = dev->data[4] = 0;

    // 2. Kirim Sinyal Start ke Sensor
    DHT_SetPinOutput(dev);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_RESET);

    // Jeda waktu start ramah multitasking (menggunakan DelayMs Anda yang berbasis vTaskDelay)
    if (dev->type == DHT11) {
        DelayMs(20); // Spesifikasi keselamatan start Adafruit untuk DHT11
    } else {
        DelayMs(2);  // Spesifikasi keselamatan start Adafruit untuk DHT22
    }

    // 3. Masuk ke mode dengar (Input Pull-up) dan tunggu pulsa selama 40us
    DHT_SetPinInput(dev);
    DelayUs(40);

    // ====================================================================
    // ENTER CRITICAL SECTION: Bekukan perpindahan task OS demi akurasi pulsa
    // ====================================================================
    taskENTER_CRITICAL();

    // 4. Deteksi Respon Awal Sensor (80us LOW dilanjutkan 80us HIGH)
    if (DHT_ExpectPulse(dev, GPIO_PIN_RESET) == UINT32_MAX) {
        taskEXIT_CRITICAL();
        dev->last_result = false;
        return DHT_TIMEOUT;
    }
    if (DHT_ExpectPulse(dev, GPIO_PIN_SET) == UINT32_MAX) {
        taskEXIT_CRITICAL();
        dev->last_result = false;
        return DHT_TIMEOUT;
    }

    // 5. Rekam 40 pulsa bit data (Sesuai cara kerja array internal Adafruit)
    uint32_t low_durations[40];
    uint32_t high_durations[40];

    for (int i = 0; i < 40; i++) {
        low_durations[i]  = DHT_ExpectPulse(dev, GPIO_PIN_RESET);
        high_durations[i] = DHT_ExpectPulse(dev, GPIO_PIN_SET);
    }

    // ====================================================================
    // EXIT CRITICAL SECTION: Bebaskan kembali OS FreeRTOS
    // ====================================================================
    taskEXIT_CRITICAL();

    // 6. Terjemahkan pulsa durasi menjadi bit 0 atau 1
    for (int i = 0; i < 40; i++) {
        if (low_durations[i] == UINT32_MAX || high_durations[i] == UINT32_MAX) {
            dev->last_result = false;
            return DHT_TIMEOUT;
        }

        dev->data[i / 8] <<= 1;
        // Algoritma Utama Adafruit: Jika durasi sinyal HIGH lebih lama dari LOW, maka nilainya bit '1'
        if (high_durations[i] > low_durations[i]) {
            dev->data[i / 8] |= 1;
        }
    }

    // 7. Validasi Checksum Integritas Data
    if (dev->data[4] != ((dev->data[0] + dev->data[1] + dev->data[2] + dev->data[3]) & 0xFF)) {
        dev->last_result = false;
        return DHT_CHECKSUM_ERROR;
    }

    // 8. Konversi Matematika Data Float (Mendukung pembacaan koma desimal DHT11)
    float f_temp = 0.0f;
    float f_hum = 0.0f;

    switch (dev->type) {
        case DHT11:
            f_hum = dev->data[0] + dev->data[1] * 0.1f;
            f_temp = dev->data[2];
            if (dev->data[3] & 0x80) {
                f_temp = -1.0f - f_temp;
            }
            f_temp += (dev->data[3] & 0x0F) * 0.1f;
            break;

        case DHT12:
            f_hum = dev->data[0] + dev->data[1] * 0.1f;
            f_temp = dev->data[2];
            f_temp += (dev->data[3] & 0x0F) * 0.1f;
            if (dev->data[2] & 0x80) {
                f_temp *= -1.0f;
            }
            break;

        case DHT21:
        case DHT22:
            f_hum = ((uint16_t)dev->data[0] << 8) | dev->data[1];
            f_hum *= 0.1f;

            f_temp = ((uint16_t)(dev->data[2] & 0x7F) << 8) | dev->data[3];
            f_temp *= 0.1f;
            if (dev->data[2] & 0x80) {
                f_temp *= -1.0f;
            }
            break;
    }

    // Simpan hasil ke cache internal perangkat
    dev->cached_data.temperature = f_temp;
    dev->cached_data.humidity = f_hum;
    dev->last_result = true;

    *output_data = dev->cached_data;
    return DHT_OK;
}
