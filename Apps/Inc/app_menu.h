/*
 * app_menu.h
 *
 *  Created on: 18 May 2026
 *      Author: ferry
 */
#ifndef INC_APP_MENU_H_
#define INC_APP_MENU_H_

#include "lcd_driver.h"
#include <stdbool.h>
#include "command_event.h"

#define LCD_ROWS    4   // untuk LCD 20x4
#define LCD_COLS    20

typedef enum {
    MENU_SENSOR,          // default tampilan sensor
    MENU_ROOT,
    MENU_DATETIME_SETTINGS,
    MENU_DATETIME_DATE,
    MENU_DATETIME_TIME,
    MENU_LOGGER_SETTINGS,
    MENU_LOGGER_INTERVAL,
    MENU_LOGGER_STORAGE,
    MENU_LOGGER_CARDINFO,
    MENU_LOGGER_FORMAT,
    MENU_NETWORK_SETTINGS,
    MENU_NETWORK_INFO,
    MENU_NETWORK_INTERVAL,
    MENU_NETWORK_SETUP,
    MENU_NETWORK_IP,
    MENU_NETWORK_MASK,
    MENU_NETWORK_GATEWAY,
    MENU_SHUTDOWN,
    MENU_EXIT
} MenuState;

typedef struct {
    const char *label;
    MenuState nextState;
} MenuItem;

void AppMenu_Init(I2C_LCDDevice *dev);
void AppMenu_Show(I2C_LCDDevice *dev);
void AppMenu_NavigateEnter(I2C_LCDDevice *dev);
void AppMenu_NavigateBack(I2C_LCDDevice *dev);
void AppMenu_MoveCursorUp(I2C_LCDDevice *dev);
void AppMenu_MoveCursorDown(I2C_LCDDevice *dev);

void AppMenu_SetState(MenuState state);
MenuState AppMenu_GetState(void);
void AppMenu_HandleEvent(I2C_LCDDevice *dev, CommandEvent *cmd);

#endif /* INC_APP_MENU_H_ */

