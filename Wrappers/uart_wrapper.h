/*
 * uart_wrapper.h
 *
 *  Created on: 29 Jun 2026
 *      Author: ferry
 */

#ifndef UART_WRAPPER_H_
#define UART_WRAPPER_H_

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    UART_HandleTypeDef *huart;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
} UART_Context;

extern UART_HandleTypeDef huart1,huart2;
extern UART_Context uart1_ctx, uart2_ctx;

void UART_Init(UART_Context *dev, UART_HandleTypeDef *huart);
HAL_StatusTypeDef UART_Send(UART_Context *dev, const uint8_t *data, uint16_t len);
HAL_StatusTypeDef UART_Receive(UART_Context *dev, uint8_t *data, uint16_t len);


#endif /* UART_WRAPPER_H_ */
