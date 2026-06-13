/*
 * my_uploader_task.h
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */

#ifndef INC_TASKS_UPLOADER_TASK_H_
#define INC_TASKS_UPLOADER_TASK_H_


#include <logger.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Deklarasi semaphore untuk sinkronisasi dengan ISR
extern SemaphoreHandle_t uploaderSemaphore;

// Prototype task
void UploaderTask(void *argument);


#endif /* INC_TASKS_UPLOADER_TASK_H_ */
