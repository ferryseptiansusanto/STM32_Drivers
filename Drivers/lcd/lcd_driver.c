/*
 * lcd_driver.c
 *
 *  Created on: 18 May 2026
 *      Author: ferry
 */

#include "lcd_driver.h"
#include "delay.h"
#include <string.h>

static uint8_t lcd_backlight = 0x08;
//static uint8_t lcd_txbuf[2];
static void LCD_SendNibble(I2C_LCDDevice *dev, uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble & 0xF0) | (rs ? LCD_RS : 0) | lcd_backlight;

    // EN = 1
    I2C_Transmit(dev->ctx, dev->address,dev->mode,(uint8_t[]){ data | LCD_EN }, 1, 50);
    DelayUs(1);

    // EN = 0
    I2C_Transmit(dev->ctx, dev->address,dev->mode, (uint8_t[]){ data & ~LCD_EN }, 1, 50);
    DelayUs(50);
}

static LCD_Status LCD_SendCmd(I2C_LCDDevice *dev, uint8_t cmd) {
    LCD_SendNibble(dev, cmd & 0xF0, 0); // RS=0
    LCD_SendNibble(dev, (cmd << 4) & 0xF0, 0);
    return LCD_OK;
}

static LCD_Status LCD_SendData(I2C_LCDDevice *dev, uint8_t dataByte) {
    LCD_SendNibble(dev, dataByte & 0xF0, 1); // RS=1
    LCD_SendNibble(dev, (dataByte << 4) & 0xF0, 1);
    return LCD_OK;
}

LCD_Status LCD_InitCmd(I2C_LCDDevice *dev) {
    // Initial reset sequence (masuk 4-bit mode)
    LCD_SendNibble(dev, 0x30, 0); // 8-bit mode
    DelayMs(5);
    LCD_SendNibble(dev, 0x30, 0);
    DelayUs(200);
    LCD_SendNibble(dev, 0x30, 0);
    DelayUs(200);
    LCD_SendNibble(dev, 0x20, 0); // switch ke 4-bit mode
    DelayUs(200);

    // Function set: 4-bit, 2 line, 5x8 dots
    LCD_SendCmd(dev, 0x28);
    DelayUs(50);

    // Display ON, cursor OFF, blink OFF
    LCD_SendCmd(dev, 0x0C);
    DelayUs(50);

    // Clear display
    LCD_SendCmd(dev, 0x01);
    DelayMs(2);

    // Entry mode: increment, no shift
    LCD_SendCmd(dev, 0x06);
    DelayUs(50);

    return LCD_OK;
}


LCD_Status LCD_Backlight(I2C_LCDDevice *dev, bool on) {
    lcd_backlight = on ? LCD_BL : 0x00;
    uint8_t data = lcd_backlight; // hanya kontrol backlight
    return (I2C_Transmit(dev->ctx, dev->address,dev->mode, &data, 1, 50) == I2C_OK) ? LCD_OK : LCD_ERROR;
}

LCD_Status LCD_TurnOff(I2C_LCDDevice *dev) {
    LCD_Clear(dev);
    LCD_PrintAt(dev, 1, 0, "LCD OFF");
    LCD_Backlight(dev, false);
    return LCD_OK;
}

LCD_Status LCD_Init(I2C_LCDDevice *dev, I2C_Context *ctx) {
	dev->ctx = ctx;
    dev->address = LCD_I2C_ADDR;   // alamat LCD dengan modul PF
    dev->sizereg = I2C_MEMADD_SIZE_8BIT; // I2C_MEMADD_SIZE_16BIT ~ EEPROM
    dev->mode = I2C_MODE_IT;
    return LCD_OK;
}


LCD_Status LCD_Clear(I2C_LCDDevice *dev) {
    if (LCD_SendCmd(dev, 0x01) != LCD_OK) return LCD_ERROR;
    DelayMs(2);
    return LCD_OK;
}

LCD_Status LCD_SetCursor(I2C_LCDDevice *dev, uint8_t row, uint8_t col) {
#ifdef LCD_OPSI_16x2
    static const uint8_t rowAddr[] = {0x00, 0x40};
#elif defined(LCD_OPSI_20x4)
    static const uint8_t rowAddr[] = {0x00, 0x40, 0x14, 0x54};
#endif

    if (row < LCD_ROWS && col < LCD_COLS) {
        uint8_t addr = 0x80 | (rowAddr[row] + col);
        return LCD_SendCmd(dev, addr);
    }
    return LCD_ERROR;
}

LCD_Status LCD_WriteChar(I2C_LCDDevice *dev, char c) {
    if (LCD_SendData(dev, (uint8_t)c) != LCD_OK) return LCD_ERROR;
    DelayUs(50);
    return LCD_OK;
}

LCD_Status LCD_WriteString(I2C_LCDDevice *dev, const char *str) {
    while (*str) {
        if (LCD_WriteChar(dev, *str++) != LCD_OK) return LCD_ERROR;
    }
    return LCD_OK;
}

LCD_Status LCD_PrintAt(I2C_LCDDevice *dev, uint8_t row, uint8_t col, const char *str) {
    if (LCD_SetCursor(dev, row, col) != LCD_OK) return LCD_ERROR;
    return LCD_WriteString(dev, str);
}

LCD_Status LCD_ClearLine(I2C_LCDDevice *dev, int row) {
    LCD_SetCursor(dev, row, 0);
    LCD_WriteString(dev, "                    "); // isi spasi sepanjang LCD_COLS
    LCD_SetCursor(dev, row, 0);
    return LCD_OK;
}
