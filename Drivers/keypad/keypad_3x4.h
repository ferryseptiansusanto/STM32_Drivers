/*
 * keypad_3x4.h
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#ifndef KEYPAD_KEYPAD_3X4_H_
#define KEYPAD_KEYPAD_3X4_H_

#include "keypad_wrapper.h"

extern const char *map4x3[4];

void KEYPAD_4x3_Init(KeypadContext *ctx, I2C_Context *i2c, uint8_t address);

#endif /* KEYPAD_KEYPAD_3X4_H_ */
