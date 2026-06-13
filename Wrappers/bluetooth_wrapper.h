/*
 * bluetooth_wrapper.h
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#ifndef BLUETOOTH_WRAPPER_H_
#define BLUETOOTH_WRAPPER_H_


#include "usart_physical.h"
#include "usart_protocol.h"

// Inisialisasi Bluetooth (pakai USART_Physical)
void BLUETOOTH_Init(USART_Physical *phy, UART_HandleTypeDef *huart);

// Kirim string (dibungkus ke frame protokol)
void BLUETOOTH_SendString(USART_Physical *phy, const char *str);

// Task TX: ambil dari queue, kirim via protokol
void BLUETOOTH_TaskTx(void *pvParameters);

// Task RX: terima dari phy, parse ke message, kirim ke queue
void BLUETOOTH_TaskRx(void *pvParameters);
#endif /* BLUETOOTH_WRAPPER_H_ */
