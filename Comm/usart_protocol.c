/*
 * usart_protocol.c
 *
 *  Created on: 13 May 2026
 *      Author: ferry
 */


// usart_protocol.c
#include "usart_protocol.h"
#include <string.h>

int USART_Protocol_Send(USART_Physical *phy, USART_Message *msg) {
    USART_Frame f;
    f.header = 0xAA;
    f.cmd = msg->cmd;
    f.len = msg->len;
    for (int i=0;i<msg->len;i++) f.payload[i] = msg->payload[i];
    return USART_Datalink_SendFrame(phy, &f);
}

int USART_Protocol_Receive(USART_Physical *phy, USART_Message *msg) {
    USART_Frame f;
    if (!USART_Datalink_ReceiveFrame(phy, &f)) return 0;
    msg->cmd = f.cmd;
    msg->len = f.len;
    for (int i=0;i<f.len;i++) msg->payload[i] = f.payload[i];
    return 1;
}

int USART_ProtocolDMA_Parse(USART_Frame *frame, USART_Message *msg) {
    msg->cmd = (USART_Command)frame->cmd;
    msg->len = frame->len;
    memcpy(msg->payload, frame->payload, msg->len);
    return 1;
}

