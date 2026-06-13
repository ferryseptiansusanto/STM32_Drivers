/*
 * keypad_task.h
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#ifndef INC_TASKS_KEYPAD_TASK_H_
#define INC_TASKS_KEYPAD_TASK_H_

#include "FreeRTOS.h"
#include <stdbool.h>
#include "task.h"
#include "queue.h"
#include "keypad_driver.h"

// Queue global untuk hasil tombol
extern QueueHandle_t keypadQueue;

/**
 * @brief Event hasil keypad (short/long press)
 */
typedef struct {
    char key;        // karakter tombol
    bool longPress;  // true = long press, false = short press
} KeypadEvent;

/**
 * @brief Membuat task keypad
 * @param ctx pointer ke KeypadContext
 * @param priority prioritas task
 */
void KEYPAD_TaskCreate(I2C_KeypadDevice *dev, UBaseType_t priority);

#endif /* INC_TASKS_KEYPAD_TASK_H_ */
