/*
 * keypad_driver.c
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */

#include "keypad_driver.h"
#include <stdio.h>
#define KEYPAD_ROW_MASK(r) ((uint8_t)(0xFF & ~(1 << (r))))
void KEYPAD_InitCmd(I2C_KeypadDevice *dev) {

    uint8_t data = 0xFF; // default semua high
    I2C_Transmit(dev->ctx, dev->address, dev->mode, &data, 1, 50);
}

bool KEYPAD_Scan(I2C_KeypadDevice *dev, uint8_t *row, uint8_t *col) {
    for (uint8_t r = 0; r < dev->rows; r++) {
        uint8_t mask =KEYPAD_ROW_MASK(r);
        uint8_t val;

        if (I2C_TransmitReceive(dev->ctx, dev->address, dev->mode, &mask, 1, 50, &val, 1, 50) != I2C_OK) {
            return false;
        }

        for (uint8_t c = 0; c < dev->cols; c++) {
            if (!(val & (1 << ( c+4 )))) {
//            	printf("Row=%d mask=0x%02X val=0x%02X\n", r, mask, val);
                *row = r;
                *col = c;
                return true;
            }
        }
    }
    return false;

}

char KEYPAD_GetKey(I2C_KeypadDevice *dev) {
    uint8_t row, col;
    if (KEYPAD_Scan(dev, &row, &col)) {
        return dev->map[row][col];
    }
    return 0; // tidak ada tombol
}

