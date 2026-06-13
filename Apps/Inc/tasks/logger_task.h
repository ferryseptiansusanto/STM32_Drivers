/*
 * my_logger_task.h
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */

#ifndef INC_TASKS_LOGGER_TASK_H_
#define INC_TASKS_LOGGER_TASK_H_

#include <ds3231_wrapper.h>
#include <logger.h>
#define TASK_LOGGER_INTERVAL 30000

typedef struct {
    char temp[16];
    char humidity[16];
    char current[16];
    char voltage[16];
} RecordData;

extern RecordData SensorData;

// Prototype task
void LOGGER_TaskCreate(SPI_Context *Storage_Ctx, I2C_RTCDevice *Rtc_Ctx, UBaseType_t priority);

#endif /* INC_TASKS_LOGGER_TASK_H_ */
