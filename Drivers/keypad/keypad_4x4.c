/*
 * keypad_4x4.c
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#include "keypad_4x4.h"

const char *map4x4[4] = {
    "123A",
    "456B",
    "789C",
    "*0#D"
};

void KEYPAD_Init(I2C_KeypadDevice *dev, I2C_Context *ctx) {
	dev->ctx = ctx;
	dev->address = KEYPAD_ADDR;
	dev->mode = I2C_MODE_IT;
	dev->map = map4x4;
	dev->cols = 4;
	dev->rows = 4;
}

