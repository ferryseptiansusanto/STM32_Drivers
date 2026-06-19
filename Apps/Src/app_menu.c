/*
 * app_menu.c
 *
 *  Created on: 18 May 2026
 *      Author: ferry
 */


#include "app_menu.h"
#include "logger_task.h"
#include "lcd_driver.h"
#include "command_event.h"
#include "datetime_handler.h"   // integrasi handler datetime
#include <stdio.h>

static int cursorPos = 0;
static int scrollOffset = 0;
static MenuState currentMenu = MENU_SENSOR;
extern I2C_LCDDevice Lcd_Ctx;
extern RecordData SensorData;

// ===== Definisi menu =====
static const MenuItem rootMenu[] = {
    { "LOGGER SETTINGS", MENU_LOGGER_SETTINGS },
    { "NETWORK SETTINGS", MENU_NETWORK_SETTINGS },
    { "DATETIME SETTINGS", MENU_DATETIME_SETTINGS },
    { "SHUTDOWN", MENU_SHUTDOWN }
};
static const MenuItem datetimeMenu[] = {
    { "SET DATE", MENU_DATETIME_DATE },
    { "SET TIME", MENU_DATETIME_TIME }
};
static const MenuItem loggerMenu[] = {
    { "INTERVAL RECORDING", MENU_LOGGER_INTERVAL },
    { "STORAGE", MENU_LOGGER_STORAGE }
};
static const MenuItem networkMenu[] = {
    { "INFO", MENU_NETWORK_INFO },
    { "INTERVAL UPLOADING", MENU_NETWORK_INTERVAL },
    { "SETUP", MENU_NETWORK_SETUP }
};

// ===== Helper mapping =====
typedef struct { const MenuItem *items; int count; } MenuTable;

static MenuTable AppMenu_GetTable(MenuState state) {
    switch(state) {
        case MENU_ROOT:             return (MenuTable){rootMenu, sizeof(rootMenu)/sizeof(rootMenu[0])};
        case MENU_DATETIME_SETTINGS:return (MenuTable){datetimeMenu, sizeof(datetimeMenu)/sizeof(datetimeMenu[0])};
        case MENU_LOGGER_SETTINGS:  return (MenuTable){loggerMenu, sizeof(loggerMenu)/sizeof(loggerMenu[0])};
        case MENU_NETWORK_SETTINGS: return (MenuTable){networkMenu, sizeof(networkMenu)/sizeof(networkMenu[0])};
        default: return (MenuTable){NULL,0};
    }
}

// ===== Tampilan sensor default =====
static void showSensor(I2C_LCDDevice *dev) {
    char buf[41];
    LCD_Clear(dev);
    LCD_SetCursor(dev,0,0);
    DS3231_DateTime now;
    now = DS3231_GetDateTime(&DS3231_Ctx);
	snprintf(buf, sizeof(buf),
			 "%02d-%02d-%04dT%02d:%02d:%02d",
			 now.date.day, now.date.month, now.date.year,
			 now.time.hours, now.time.minutes, now.time.seconds);
    LCD_WriteString(dev,buf);
    LCD_SetCursor(dev,1,0);
    snprintf(buf,sizeof(buf),"TEMP:%sC HUM:%s%%",SensorData.temp,SensorData.humidity);
    LCD_WriteString(dev,buf);
    LCD_SetCursor(dev,2,0);
    LCD_WriteString(dev,"DEVICE READY");
}

// ===== Tampilan menu =====
void AppMenu_Show(I2C_LCDDevice *dev) {
    if (currentMenu == MENU_SENSOR) { showSensor(dev); return; }
    MenuTable tbl = AppMenu_GetTable(currentMenu);
    LCD_Clear(dev);

    if (tbl.items) {
        for (int row=0; row<LCD_ROWS && (scrollOffset+row)<tbl.count; row++) {
            LCD_SetCursor(dev,row,0);
            LCD_WriteString(dev,(scrollOffset+row)==cursorPos?">":" ");
            LCD_WriteString(dev,tbl.items[scrollOffset+row].label);
        }
    } else {
        LCD_SetCursor(dev,0,0);
        switch(currentMenu) {
            case MENU_DATETIME_DATE: LCD_WriteString(dev,"SET DATE"); break;
            case MENU_DATETIME_TIME: LCD_WriteString(dev,"SET TIME"); break;
            default: LCD_WriteString(dev,"ACTION"); break;
        }
    }
}

// ===== Navigasi =====
void AppMenu_NavigateEnter(I2C_LCDDevice *dev) {
    MenuTable tbl = AppMenu_GetTable(currentMenu);
    if (tbl.items) {
        currentMenu = tbl.items[cursorPos].nextState;
        cursorPos = scrollOffset = 0;
        AppMenu_Show(dev);
    }
}

void AppMenu_NavigateBack(I2C_LCDDevice *dev) {
    switch(currentMenu) {
        case MENU_ROOT: currentMenu = MENU_SENSOR; break;
        case MENU_DATETIME_DATE:
        case MENU_DATETIME_TIME: currentMenu = MENU_DATETIME_SETTINGS; break;
        case MENU_DATETIME_SETTINGS:
        case MENU_LOGGER_SETTINGS:
        case MENU_NETWORK_SETTINGS:
        case MENU_SHUTDOWN: currentMenu = MENU_ROOT; break;
        default: currentMenu = MENU_ROOT; break;
    }
    cursorPos = scrollOffset = 0;
    AppMenu_Show(dev);
}

void AppMenu_MoveCursorUp(I2C_LCDDevice *dev) {
    if (cursorPos>0) { cursorPos--; if (cursorPos<scrollOffset) scrollOffset--; }
    AppMenu_Show(dev);
}

void AppMenu_MoveCursorDown(I2C_LCDDevice *dev) {
    int count = AppMenu_GetTable(currentMenu).count;
    if (cursorPos<count-1) { cursorPos++; if (cursorPos>=scrollOffset+LCD_ROWS) scrollOffset++; }
    AppMenu_Show(dev);
}

// ===== API =====
void AppMenu_Init(I2C_LCDDevice *dev) {
    cursorPos = scrollOffset = 0;
    currentMenu = MENU_SENSOR;
    AppMenu_Show(dev);
}

void AppMenu_SetState(MenuState state) {
    currentMenu = state;
    cursorPos = scrollOffset = 0;
}

MenuState AppMenu_GetState(void) { return currentMenu; }

void AppMenu_HandleEvent(I2C_LCDDevice *dev, CommandEvent *cmd) {
    if (cmd->type == CMD_KEYPAD) {
        // delegasi ke handler sesuai state
        switch(currentMenu) {
            case MENU_DATETIME_DATE:
            case MENU_DATETIME_TIME:
                Datetime_HandleInput(dev, cmd);
                return;
            default:
                break;
        }
        // navigasi umum
        switch(cmd->data.keypad.key) {
            case KEY_MENU:   AppMenu_SetState(MENU_ROOT); AppMenu_Show(dev); break;
            case KEY_BACK:   AppMenu_NavigateBack(dev); break;
            case KEY_ENTER:  if (!cmd->data.keypad.longPress) AppMenu_NavigateEnter(dev); break;
            case KEY_UP:
            case KEY_LEFT:   AppMenu_MoveCursorUp(dev); break;
            case KEY_DOWN:
            case KEY_RIGHT:  AppMenu_MoveCursorDown(dev); break;
            default: break;
        }
    }
}



