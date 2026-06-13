/*
 * lcd_task.c
 *
 *  Created on: 19 May 2026
 *      Author: ferry
 */


#include "lcd_driver.h"
#include "app_menu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"

extern I2C_LCDDevice Lcd_Ctx;

static void vTaskLcd(void *pvParameters) {
    // Step 1: Init LCD (binding context)
    LCD_InitCmd(&Lcd_Ctx);
    LCD_Backlight(&Lcd_Ctx, true);

    // Step 2: Kirim sequence init + pesan sambutan
    // --- Tambahkan pesan sambutan ---
    LCD_SetCursor(&Lcd_Ctx, 0, 0);
    LCD_WriteString(&Lcd_Ctx, "WELCOME");
    LCD_SetCursor(&Lcd_Ctx, 1, 0);
    LCD_WriteString(&Lcd_Ctx, "LOGGER READY");

    // Step 3: Tampilkan pesan sambutan beberapa detik
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Step 4: Clear dan masuk ke menu default (sensor)
//    AppMenu_Init(&Lcd_Ctx);

    // Step 5: Task bisa dihapus (dummy hanya sekali jalan)
    vTaskDelete(NULL);
}

void LCD_TaskCreate(UBaseType_t priority) {
    xTaskCreate(vTaskLcd, "LcdTask", 512, NULL, priority, NULL);
}


