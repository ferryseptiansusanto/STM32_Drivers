/*
 * keypad_3x4.c
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */


#include "keypad_4x3.h"

const char *map4x3[4] = {
    "123",
    "456",
    "789",
    "*0#"
};

void KEYPAD_4x3_Init(KeypadContext *ctx, I2C_Context *i2c, uint8_t address) {
    ctx->hw.i2c = i2c;
    ctx->hw.i2c->address = address;
    ctx->hw.i2c->mode = I2C_MODE_IT;
    ctx->hw.rows = 4;
    ctx->hw.cols = 3;
    ctx->map = map4x3;
    KEYPAD_Init(ctx);
}

