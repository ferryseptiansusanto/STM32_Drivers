/*
 * lcd_driver.h
 *
 *  Created on: 9 May 2026
 *      Author: ferry
 */
/*
 * lcd_driver.h
 *
 *  Created on: 18 May 2026
 *      Author: ferry
 */

#ifndef LCD_LCD_DRIVER_H_
#define LCD_LCD_DRIVER_H_

#include "stm32f1xx_hal.h"
#include "i2c_wrapper.h"
#include <stdint.h>
#include <stdbool.h>

// ===== OPSI LCD =====
//#define LCD_OPSI_16x2
#define LCD_OPSI_20x4

#ifdef LCD_OPSI_16x2
    #define LCD_ROWS   2
    #define LCD_COLS   16
#elif defined(LCD_OPSI_20x4)
    #define LCD_ROWS   4
    #define LCD_COLS   20
#endif

// Alamat I2C modul backpack (PCF8574)
#define LCD_I2C_ADDR   (0x27 << 1)
#define LCD_RS  (1<<0)
#define LCD_RW  (1<<1)
#define LCD_EN  (1<<2)
#define LCD_BL  (1<<3)   // atau (1<<7) tergantung modul

typedef struct {
	I2C_Context *ctx;
    uint16_t address;
    uint32_t sizereg;
    I2C_Mode mode;
} I2C_LCDDevice;

// Status LCD
typedef enum {
    LCD_OK = 0,
    LCD_ERROR,
    LCD_TIMEOUT
} LCD_Status;

// ===== API =====
LCD_Status LCD_Init(I2C_LCDDevice *dev, I2C_Context *ctx);
LCD_Status LCD_InitCmd(I2C_LCDDevice *dev);
LCD_Status LCD_Clear(I2C_LCDDevice *dev);

LCD_Status LCD_SetCursor(I2C_LCDDevice *dev, uint8_t row, uint8_t col);
LCD_Status LCD_WriteChar(I2C_LCDDevice *dev, char c);
LCD_Status LCD_WriteString(I2C_LCDDevice *dev, const char *str);
LCD_Status LCD_PrintAt(I2C_LCDDevice *dev, uint8_t row, uint8_t col, const char *str);
LCD_Status LCD_Backlight(I2C_LCDDevice *dev, bool on);
LCD_Status LCD_TurnOff(I2C_LCDDevice *dev);
LCD_Status LCD_ClearLine(I2C_LCDDevice *dev, int row);

#endif /* LCD_LCD_DRIVER_H_ */

