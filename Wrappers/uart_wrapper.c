/*
 * uart_wrapper.c
 *
 *  Created on: 29 Jun 2026
 *      Author: ferry
 */
#include "uart_wrapper.h"
#define UART_TIMEOUT_MS 1000

UART_Context uart1_ctx, uart2_ctx = { &huart2, NULL, NULL};

void UART_Init(UART_Context *dev, UART_HandleTypeDef *huart) {
	dev->huart = huart;
	dev->tx_sem = xSemaphoreCreateBinary();
	dev->rx_sem = xSemaphoreCreateBinary();
}

HAL_StatusTypeDef UART_Send(UART_Context *dev, const uint8_t *data, uint16_t len) {
    if (HAL_UART_Transmit_DMA(dev->huart, (uint8_t*)data, len) != HAL_OK) return HAL_ERROR;
    if (xSemaphoreTake(dev->tx_sem, pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdPASS) return HAL_TIMEOUT;
    return HAL_OK;
}

HAL_StatusTypeDef UART_Receive(UART_Context *dev, uint8_t *data, uint16_t len) {
    if (HAL_UART_Receive_DMA(dev->huart, data, len) != HAL_OK) return HAL_ERROR;
    if (xSemaphoreTake(dev->rx_sem, pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdPASS) return HAL_TIMEOUT;
    return HAL_OK;
}

// --- Callback DMA ---

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart == uart1_ctx.huart) xSemaphoreGiveFromISR(uart1_ctx.tx_sem, &xHigherPriorityTaskWoken);
    else if (huart == uart2_ctx.huart) xSemaphoreGiveFromISR(uart2_ctx.tx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart == uart1_ctx.huart) xSemaphoreGiveFromISR(uart1_ctx.rx_sem, &xHigherPriorityTaskWoken);
    else if (huart == uart2_ctx.huart) xSemaphoreGiveFromISR(uart2_ctx.rx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
