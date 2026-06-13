/*
 * keypad_driver.h
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#ifndef KEYPAD_KEYPAD_DRIVER_H_
#define KEYPAD_KEYPAD_DRIVER_H_

#include "i2c_wrapper.h"
#include <stdbool.h>
#define KEYPAD_I2C_ADDR   (0x20 << 1)

typedef struct {
    I2C_Context *ctx;   // pointer ke context I²C
    uint16_t address;
    I2C_Mode mode;
    const char **map;
    uint8_t rows;       // jumlah baris
    uint8_t cols;       // jumlah kolom
} I2C_KeypadDevice;



void KEYPAD_InitCmd(I2C_KeypadDevice *dev);
bool KEYPAD_Scan(I2C_KeypadDevice *dev, uint8_t *row, uint8_t *col);
char KEYPAD_GetKey(I2C_KeypadDevice *dev);


#endif /* KEYPAD_KEYPAD_DRIVER_H_ */
