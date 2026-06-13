#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#ifdef STM32F1xx
#include "stm32f1xx.h"
#endif


uint32_t GetTick(void);
// Delay dalam milidetik (non-blocking, multitasking friendly)
void DelayMs(uint32_t ms);

// Delay mikrodetik (blocking, hanya untuk kebutuhan singkat)
void DelayUs(uint32_t us);

#endif /* DELAY_H_ */
