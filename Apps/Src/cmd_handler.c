/*
 * cmd_handler.c
 *
 *  Created on: 13 May 2026
 *      Author: ferry
 */


// usart_application.c
#include <cmd_handler.h>
#include <stdio.h>

void USART_App_HandleMessage(USART_Message *msg) {
    switch (msg->cmd) {
        case CMD_SET_PARAM:
            // contoh: set parameter
            printf("Set param: %d\n", msg->payload[0]);
            break;
        case CMD_GET_DATA:
            printf("Request data\n");
            break;
        case CMD_STREAM_ON:
            printf("Start streaming\n");
            break;
        case CMD_STREAM_OFF:
            printf("Stop streaming\n");
            break;
        default:
            printf("Unknown command\n");
            break;
    }
}
