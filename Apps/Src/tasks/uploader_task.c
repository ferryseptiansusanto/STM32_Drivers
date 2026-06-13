/*
 * my_uploader_task.c
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */


#include "logging.h"
#include "uploader_task.h"

// Semaphore didefinisikan di main.c
extern SemaphoreHandle_t uploaderSemaphore;

void UploaderTask(void *argument) {
    for (;;) {
        // Tunggu semaphore dari ISR DS3231 (alarm 1 jam)
        if (xSemaphoreTake(uploaderSemaphore, portMAX_DELAY) == pdTRUE) {
            logInfo("UPLOADER", "Starting upload...");

            // Panggil fungsi upload dari my_logger.c
            bool success = Logger_Upload("server.com", 80, "token123");

            if (success) {
                logInfo("UPLOADER", "Upload success");
            } else {
                logError("UPLOADER", "Upload failed");
                // Bisa tambahkan retry mechanism di sini
            }
        }
    }
}
