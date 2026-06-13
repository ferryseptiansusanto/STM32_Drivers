
/*
 * sensor.c
 *
 *  Created on: 30 Apr 2026
 *      Author: ferry
 */

#include <flow_sensor.h>
#include <kwh_sensor.h>
#include <ph_sensor.h>
#include <pressure_sensor.h>
#include <temp_sensor.h>
#include "sensor.h"

bool Sensor_Init(void) {
    // Inisialisasi semua sensor yang ada
    TempSensor_Init();
    PhSensor_Init();
    PressureSensor_Init();
    FlowSensor_Init();
    KwhSensor_Init();
    return true;
}

bool Sensor_Deinit(void) {
    // optional: release resource
    return true;
}

bool Sensor_Read(SensorType_t type, SensorData_t *data) {
    if (!data) return false;

    switch (type) {
        case SENSOR_TEMP:
            data->value = TempSensor_Read();
            data->type  = SENSOR_TEMP;
            data->flag  = 0;
            return true;

        case SENSOR_PH:
            data->value = PhSensor_Read();
            data->type  = SENSOR_PH;
            data->flag  = 0;
            return true;

        case SENSOR_PRESSURE:
            data->value = PressureSensor_Read();
            data->type  = SENSOR_PRESSURE;
            data->flag  = 0;
            return true;

        case SENSOR_FLOW:
            data->value = FlowSensor_Read();
            data->type  = SENSOR_FLOW;
            data->flag  = 0;
            return true;

        case SENSOR_KWH:
            data->value = KwhSensor_Read();
            data->type  = SENSOR_KWH;
            data->flag  = 0;
            return true;

        default:
            data->value = 0.0f;
            data->type  = SENSOR_UNKNOWN;
            data->flag  = 0xFFFFFFFF;
            return false;
    }
}
