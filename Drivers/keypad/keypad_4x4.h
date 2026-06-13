/*
 * keypad_4x4.h
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#ifndef KEYPAD_KEYPAD_4X4_H_
#define KEYPAD_KEYPAD_4X4_H_

#include "keypad_driver.h"
#define KEYPAD_ADDR 0x20<<1

extern const char *map4x4[4];

void KEYPAD_Init(I2C_KeypadDevice *dev, I2C_Context *ctx);

#endif /* KEYPAD_KEYPAD_4X4_H_ */
