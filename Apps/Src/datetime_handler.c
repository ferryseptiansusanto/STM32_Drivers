/*
 * datetime_handler.c
 *
 *  Created on: 11 Jun 2026
 *      Author: ferry
 */


#include "datetime_handler.h"
#include "app_menu.h"
#include <string.h>
#include <stdio.h>

static char inputBuffer[16];
static int inputIndex = 0;

extern I2C_RTCDevice DS3231_Ctx;

void Datetime_HandleInput(I2C_LCDDevice *dev, CommandEvent *cmd) {
    char k = cmd->data.keypad.key;

    // Input angka
    if (k >= '0' && k <= '9') {
        if (inputIndex < sizeof(inputBuffer)-1) {
            inputBuffer[inputIndex++] = k;
            inputBuffer[inputIndex] = '\0';

            // Tambahkan separator otomatis
            if (AppMenu_GetState() == MENU_DATETIME_DATE) {
                if (inputIndex == 2 || inputIndex == 5) {
                    inputBuffer[inputIndex++] = '/';
                    inputBuffer[inputIndex] = '\0';
                }
            } else if (AppMenu_GetState() == MENU_DATETIME_TIME) {
                if (inputIndex == 2 || inputIndex == 5) {
                    inputBuffer[inputIndex++] = ':';
                    inputBuffer[inputIndex] = '\0';
                }
            }

            LCD_SetCursor(dev,1,0);
            LCD_WriteString(dev,"                "); // clear line
            LCD_SetCursor(dev,1,0);
            LCD_WriteString(dev,inputBuffer);
        }
    }
    // ENTER → parsing & set RTC
    else if (k == KEY_ENTER && !cmd->data.keypad.longPress) {
        if (AppMenu_GetState() == MENU_DATETIME_DATE && strlen(inputBuffer)==10) {
            DS3231_Date d;
            d.day   = (inputBuffer[0]-'0')*10 + (inputBuffer[1]-'0');
            d.month = (inputBuffer[3]-'0')*10 + (inputBuffer[4]-'0');
            d.year  = (inputBuffer[6]-'0')*1000 + (inputBuffer[7]-'0')*100 + (inputBuffer[8]-'0')*10 + (inputBuffer[9]-'0');
            DS3231_SetDate(&DS3231_Ctx, d);
            LCD_ClearLine(dev, 0);
            LCD_WriteString(dev, "Date Set OK");
            // kembali ke menu utama
            AppMenu_SetState(MENU_DATETIME_SETTINGS);
            AppMenu_Show(dev);
        }
        else if (AppMenu_GetState() == MENU_DATETIME_TIME && strlen(inputBuffer)==8) {
            DS3231_Time t;
            t.hours   = (inputBuffer[0]-'0')*10 + (inputBuffer[1]-'0');
            t.minutes = (inputBuffer[3]-'0')*10 + (inputBuffer[4]-'0');
            t.seconds = (inputBuffer[6]-'0')*10 + (inputBuffer[7]-'0');
            DS3231_SetTime(&DS3231_Ctx, t);
            LCD_ClearLine(dev, 0);
            LCD_WriteString(dev,"Time Set OK");
            // kembali ke menu utama
            AppMenu_SetState(MENU_DATETIME_SETTINGS);
            AppMenu_Show(dev);
        } else {
            LCD_WriteString(dev,"Invalid Input");
        }
        inputIndex = 0;
        inputBuffer[0] = '\0';
    }
    // BACK → hapus karakter terakhir (termasuk separator)
    else if (k == KEY_BACK) {
        if (inputIndex>0) {
            inputIndex--;
            if ((AppMenu_GetState()==MENU_DATETIME_DATE && (inputIndex==2 || inputIndex==5)) ||
                (AppMenu_GetState()==MENU_DATETIME_TIME && (inputIndex==2 || inputIndex==5))) {
                inputIndex--;
            }
            inputBuffer[inputIndex] = '\0';
            LCD_SetCursor(dev,1,0);
            LCD_WriteString(dev,"                ");
            LCD_SetCursor(dev,1,0);
            LCD_WriteString(dev,inputBuffer);
        }
    }
}
