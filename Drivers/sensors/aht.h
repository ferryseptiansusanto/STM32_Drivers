/*
 * AHT.h
 *
 *  Created on: 19 Jun 2026
 *      Author: ferry
 */

#ifndef SENSORS_AHT_H_
#define SENSORS_AHT_H_

/*
 * AHT.h
 *
 * Created on: 19 Jun 2026
 * Author: ferry
 */

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define AHT_I2C_ADDRESS    (0x38 << 1) // Alamat I2C standar AHT (7-bit digeser ke kiri)
#define AHT_MIN_INTERVAL   1000        // Jeda minimal pembacaan aman 1 detik ala Adafruit

// Definisi Command AHT
#define AHT_CMD_INITIALIZE 0xE1
#define AHT_CMD_TRIGGER    0xAC
#define AHT_CMD_SOFTRESET  0xBA

// Definisi Bit Status AHT
#define AHT_STATUS_BUSY    0x80 // Bit 7: 1 = Sibuk, 0 = Siap
#define AHT_STATUS_CALBOOT 0x08 // Bit 3: 1 = Terkalibrasi, 0 = Belum

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
    I2C_HandleTypeDef *hi2c;   // Menggunakan hardware I2C handle, bukan GPIO

    // Variabel untuk algoritma cache Adafruit & FreeRTOS
    uint8_t data[6];           // Buffer data mentah (1 byte status + 5 bytes data)
    uint32_t last_read_time;   // Menyimpan tick waktu pembacaan terakhir
    bool last_result;          // Menyimpan status keberhasilan terakhir
    AHT_Data cached_data;    // Menyimpan cache hasil konversi terakhir
} AHT_Device;

// API
AHT_Status AHT_Init(AHT_Device *dev, I2C_HandleTypeDef *hi2c);
AHT_Status AHT_Read(AHT_Device *dev, AHT_Data *data, bool force);


#endif /* SENSORS_AHT_H_ */
