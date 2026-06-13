/*
 * usart_physical.h
 *
 *  Created on: 13 May 2026
 *      Author: ferry
 */

#ifndef USART_USART_PHYSICAL_H_
#define USART_USART_PHYSICAL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    UART_HandleTypeDef *huart;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
} USART_Physical;

void USART_Physical_Init(USART_Physical *phy, UART_HandleTypeDef *huart);
HAL_StatusTypeDef USART_Physical_Send(USART_Physical *phy, const uint8_t *data, uint16_t len);
HAL_StatusTypeDef USART_Physical_Receive(USART_Physical *phy, uint8_t *data, uint16_t len);

#endif /* USART_USART_PHYSICAL_H_ */
