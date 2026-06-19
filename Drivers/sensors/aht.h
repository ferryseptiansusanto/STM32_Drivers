/*
 * AHT.h
 *
 * Created on: 19 Jun 2026
 * Author: ferry
 */

#ifndef SENSORS_AHT_H_
#define SENSORS_AHT_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "i2c_wrapper.h"

#define AHT_I2C_ADDRESS         (0x38 << 1)
#define AHT_MIN_INTERVAL_MS     1000

#define AHT_CMD_INITIALIZE      0xE1
#define AHT_CMD_TRIGGER         0xAC
#define AHT_CMD_SOFTRESET       0xBA

#define AHT_STATUS_BUSY         0x80
#define AHT_STATUS_CALBOOT      0x08

typedef enum {
    AHT_OK = 0,
    AHT_ERROR,
    AHT_TIMEOUT
} AHT_Status;

typedef struct {
    float temperature;
    float humidity;
} AHT_Data;

typedef struct {
    I2C_Context *i2c_ctx;                   // Merekatkan wrapper I2C Anda
    I2C_Mode mode;                          // Menyimpan mode transfer (IT, DMA, dll)
    uint8_t raw_buffer[6];                  // Buffer data mentah 6 Byte
    uint32_t last_read_time;                // Sinkron dengan GetTick() Anda
    AHT_Data cached_data;
    bool last_result;
} AHT_Device;

/* --- API dengan 2 Parameter pada Init --- */

/**
 * @brief Inisialisasi sensor AHT, hubungkan dengan I2C Context wrapper, dan set default mode.
 * @param dev Pointer ke struktur AHT_Device lokal/global.
 * @param ctx Pointer ke struktur I2C_Context dari wrapper (misal: &i2c1_ctx atau &i2c2_ctx).
 * @return AHT_OK jika sukses, AHT_ERROR jika gagal terkoneksi.
 */
AHT_Status AHT_Init(AHT_Device *dev, I2C_Context *ctx);

/**
 * @brief Membaca data dari sensor secara non-blocking.
 */
AHT_Status AHT_Read(AHT_Device *dev, AHT_Data *output_data, bool force);

#endif /* SENSORS_AHT_H_ */
