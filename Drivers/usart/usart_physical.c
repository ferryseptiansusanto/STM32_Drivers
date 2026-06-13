/*
 * usart_physical.c
 *
 *  Created on: 13 May 2026
 *      Author: ferry
 */


// usart_physical.c
#include "usart_physical.h"
#define UART_TIMEOUT_MS 1000

void USART_Physical_Init(USART_Physical *phy, UART_HandleTypeDef *huart) {
    phy->huart = huart;
    phy->tx_sem = xSemaphoreCreateBinary();
    phy->rx_sem = xSemaphoreCreateBinary();
}

HAL_StatusTypeDef USART_Physical_Send(USART_Physical *phy, const uint8_t *data, uint16_t len) {
    if (HAL_UART_Transmit_DMA(phy->huart, (uint8_t*)data, len) != HAL_OK) return HAL_ERROR;
    if (xSemaphoreTake(phy->tx_sem, pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdPASS) return HAL_TIMEOUT;
    return HAL_OK;
}

HAL_StatusTypeDef USART_Physical_Receive(USART_Physical *phy, uint8_t *data, uint16_t len) {
    if (HAL_UART_Receive_DMA(phy->huart, data, len) != HAL_OK) return HAL_ERROR;
    if (xSemaphoreTake(phy->rx_sem, pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdPASS) return HAL_TIMEOUT;
    return HAL_OK;
}

// --- Callback DMA ---
extern USART_Physical usart1_phy, usart2_phy;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart == usart1_phy.huart) xSemaphoreGiveFromISR(usart1_phy.tx_sem, &xHigherPriorityTaskWoken);
    else if (huart == usart2_phy.huart) xSemaphoreGiveFromISR(usart2_phy.tx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart == usart1_phy.huart) xSemaphoreGiveFromISR(usart1_phy.rx_sem, &xHigherPriorityTaskWoken);
    else if (huart == usart2_phy.huart) xSemaphoreGiveFromISR(usart2_phy.rx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
