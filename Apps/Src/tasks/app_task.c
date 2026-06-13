#include "app_task.h"
#include "app_menu.h"
#include "lcd_driver.h"
#include "command_event.h"

extern I2C_LCDDevice Lcd_Ctx;
QueueHandle_t appQueue;

static bool lcdShutdown = false;
static TickType_t lastKeypadTick = 0;

static void vTaskApp(void *pvParameters) {
    CommandEvent cmd;
    static TickType_t lastUpdate = 0;
    // --- LCD Init + pesan sambutan ---
    LCD_InitCmd(&Lcd_Ctx);
    LCD_Backlight(&Lcd_Ctx, true);
    LCD_SetCursor(&Lcd_Ctx, 0, 0);
    LCD_WriteString(&Lcd_Ctx, "WELCOME");
    LCD_SetCursor(&Lcd_Ctx, 1, 0);
    LCD_WriteString(&Lcd_Ctx, "LOGGER READY");
    vTaskDelay(pdMS_TO_TICKS(5000));
    LCD_Clear(&Lcd_Ctx);

    // --- Masuk ke menu default ---
    AppMenu_Init(&Lcd_Ctx);
    lastKeypadTick = xTaskGetTickCount();

    // --- Loop utama ---
    while (1) {
        if (xQueueReceive(appQueue, &cmd, pdMS_TO_TICKS(100))) {
            lastKeypadTick = xTaskGetTickCount();

            // jika LCD sebelumnya shutdown, hidupkan kembali
            if (lcdShutdown) {
                lcdShutdown = false;
//                LCD_Clear(&Lcd_Ctx);
                LCD_Backlight(&Lcd_Ctx, true);
                AppMenu_Show(&Lcd_Ctx);
            }

            // delegasi ke procedure di app_menu
            AppMenu_HandleEvent(&Lcd_Ctx, &cmd);
        }

        // refresh sensor setiap 1 detik
        if (AppMenu_GetState() == MENU_SENSOR) {
            TickType_t now = xTaskGetTickCount();
            if (now - lastUpdate >= pdMS_TO_TICKS(1000)) {
                lastUpdate = now;   // update hanya setelah refresh
                AppMenu_Show(&Lcd_Ctx);
            }
        }

        // idle timeout → matikan LCD
        if (!lcdShutdown && (xTaskGetTickCount() - lastKeypadTick > pdMS_TO_TICKS(15000))) {
            lcdShutdown = true;
            LCD_TurnOff(&Lcd_Ctx);
        }
    }
}

void APP_TaskCreate(UBaseType_t priority) {
    appQueue = xQueueCreate(10, sizeof(CommandEvent));
    xTaskCreate(vTaskApp, "AppTask", 512, NULL, priority, NULL);
}
