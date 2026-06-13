/*
 * sensor.h
 *
 *  Created on: 30 Apr 2026
 *      Author: ferry
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <stdint.h>
#include <stdbool.h>

// Struktur umum untuk data sensor
typedef struct {
    float value;       // nilai utama sensor (misalnya suhu, pH, tekanan)
    uint8_t type;      // tipe sensor (enum)
    uint32_t flag;     // status atau flag tambahan
} SensorData_t;

// Enum tipe sensor
typedef enum {
    SENSOR_TEMP = 0,
    SENSOR_PH,
    SENSOR_PRESSURE,
    SENSOR_FLOW,
    SENSOR_KWH,
    SENSOR_UNKNOWN
} SensorType_t;

// API umum
bool Sensor_Init(void);
bool Sensor_Deinit(void);

bool Sensor_Read(SensorType_t type, SensorData_t *data);

#endif /* SENSOR_H_ */
