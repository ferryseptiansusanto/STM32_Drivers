/*
 * bluetooth_wrapper.c
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */
#include "bluetooth_wrapper.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>

extern QueueHandle_t btQueueTx;
extern QueueHandle_t btQueueRx;
extern QueueHandle_t uartQueue; // queue utama untuk command_task

void BLUETOOTH_Init(USART_Physical *phy, UART_HandleTypeDef *huart) {
    USART_Physical_Init(phy, huart);
}

void BLUETOOTH_SendString(USART_Physical *phy, const char *str) {
    USART_Message msg;
    msg.cmd = CMD_SET_PARAM;   // contoh command
    msg.len = strlen(str);
    memcpy(msg.payload, str, msg.len);
    USART_Protocol_Send(phy, &msg);
}

void BLUETOOTH_TaskTx(void *pvParameters) {
    USART_Physical *phy = (USART_Physical*)pvParameters;
    char data[64];
    while (1) {
        if (xQueueReceive(btQueueTx, &data, portMAX_DELAY)) {
            BLUETOOTH_SendString(phy, data);
        }
    }
}

void BLUETOOTH_TaskRx(void *pvParameters) {
    USART_Physical *phy = (USART_Physical*)pvParameters;
    USART_Message msg;
    while (1) {
        if (USART_Protocol_Receive(phy, &msg)) {
            // kirim ke queue utama supaya command_task bisa eksekusi
            xQueueSend(uartQueue, &msg, portMAX_DELAY);
        }
    }
}
