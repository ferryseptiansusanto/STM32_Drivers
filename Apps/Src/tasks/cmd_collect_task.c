/*
 * cmd_collect_task.c
 *
 *  Created on: 13 May 2026
 *      Author: ferry
 */

#include <cmd_handler.h>   // untuk USART_AppDMA_HandleMessage jika diperlukan
#include "cmd_collect_task.h"

extern USART_Physical usart1_phy;   // didefinisikan di main.c
static QueueHandle_t cmdQueue;      // queue handle untuk command

void CmdCollectTaskInit(QueueHandle_t queue) {
    cmdQueue = queue;
    xTaskCreate(cmd_collect_task, "cmd_collect_task", 256, NULL, 2, NULL);
}

void cmd_collect_task(void *argument) {
    USART_Frame frame;
    USART_Message msg;
    uint8_t buf[FRAME_MAX_LEN + 4];

    for (;;) {
        // Ambil data dari UART (DMA + semaphore)
        if (USART_Physical_Receive(&usart1_phy, buf, sizeof(buf)) == HAL_OK) {
            // Parsing buffer ke frame
            if (USART_DatalinkDMA_ParseBuffer(buf, sizeof(buf), &frame)) {
                // Parsing frame ke message
                if (USART_ProtocolDMA_Parse(&frame, &msg)) {
                    // Kirim message ke queue
                    xQueueSend(cmdQueue, &msg, portMAX_DELAY);
                }
            }
        }
    }
}
