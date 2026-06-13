#ifndef INC_TASKS_APP_TASK_H_
#define INC_TASKS_APP_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "command_event.h"
#include "lcd_driver.h"

// Queue untuk komunikasi dari command_task → app_task
extern QueueHandle_t appQueue;

// Membuat task aplikasi (menu + LCD)
void APP_TaskCreate(UBaseType_t priority);

#endif /* INC_TASKS_APP_TASK_H_ */
