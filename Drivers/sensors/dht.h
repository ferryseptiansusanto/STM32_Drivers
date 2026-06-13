/*
 * dht.h
 *
 *  Created on: 9 Jun 2026
 *      Author: ferry
 */

#ifndef SENSORS_DHT_H_
#define SENSORS_DHT_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define MIN_INTERVAL 2000 // Jeda minimal 2 detik standar Adafruit

typedef enum {
    DHT_OK = 0,
    DHT_ERROR,
    DHT_TIMEOUT,
    DHT_CHECKSUM_ERROR
} DHT_Status;

typedef struct {
    float temperature;
    float humidity;
} DHT_Data;

typedef enum {
    DHT11,
	DHT12,
	DHT21,
    DHT22
} DHT_Type;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    DHT_Type type;

    // Tambahan variabel untuk algoritma cache Adafruit & FreeRTOS
    uint8_t data[5];          // Penampung buffer data mentah 40-bit
    uint32_t last_read_time;   // Menyimpan tick waktu pembacaan terakhir
    bool last_result;          // Menyimpan status keberhasilan terakhir
    DHT_Data cached_data;      // Menyimpan cache hasil konversi terakhir
} DHT_Device;

// API
void DHT_Init(DHT_Device *dev, GPIO_TypeDef *port, uint16_t pin, DHT_Type type);
DHT_Status DHT_Read(DHT_Device *dev, DHT_Data *data, bool force);

#endif /* SENSORS_DHT_H_ */
