/*
 * my_logger_task.c (refactored with wrappers)
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */

#include <current_sensor.h>
#include "dht.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "storage.h"
#include "logger.h"
#include "logger_task.h"
#include "ds3231_wrapper.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <temp_sensor.h>
#include <voltage_sensor.h>

typedef struct {
    SPI_Context *storage;
    I2C_RTCDevice *rtc;
} LoggerParams;



RecordData SensorData;

extern SemaphoreHandle_t SD_Card_Mutex;
extern I2C_HandleTypeDef hi2c1;   // handle I2C untuk DS3231
extern DHT_Device dht;

void vTaskLogger(void *pvParameters) {
    LoggerParams *params = (LoggerParams*)pvParameters;
    SPI_Context *Storage_Ctx = params->storage;
    I2C_RTCDevice *Rtc_Ctx = params->rtc;

    DS3231_DateTime now;
    char filename[32];
    char line[256];
    char currentFile[32] = "";

    for (;;) {
     //   if (xSemaphoreTake(SD_Card_Mutex, portMAX_DELAY) == pdTRUE) {
            // 1. Pastikan card ada
            if (!STORAGE_IsCardPresent(Storage_Ctx)) {
                LOG_Close();
                printf("LoggerTask: SD card removed\r\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // 2. Inisialisasi storage jika belum siap
            if (STORAGE_GetStatus(Storage_Ctx) != STORAGE_OK) {
                if (LOG_Init() == LOG_OK) {
                    printf("LoggerTask: SD card initialized\r\n");
                } else {
                    printf("LoggerTask: Init failed\r\n");
                    LOG_Close();
                    LOG_Unmount();
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                }
            }

            // 3. Ambil waktu dari DS3231
            now = DS3231_GetDateTime(Rtc_Ctx);

            // 4. Buat nama file sesuai tanggal hari ini (YYYYMMDD.csv)
            snprintf(filename, sizeof(filename),
                     "%04d%02d%02d.csv",
                     now.date.year, now.date.month, now.date.day);

            // 5. Jika berganti hari → rotate file
            if (strcmp(filename, currentFile) != 0) {
                LOG_Close();
                strcpy(currentFile, filename);
                if (LOG_Open(currentFile) == LOG_OK) {
                    printf("LoggerTask: Log file rotated to %s\r\n", currentFile);
                } else {
                    printf("LoggerTask: Failed to open new file\r\n");
                    LOG_Close();
//                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                }
            }

            // 6. Cek status kartu sebelum menulis
            if (STORAGE_GetStatus(Storage_Ctx) != STORAGE_OK) {
                printf("LoggerTask: Card not ready\r\n");

                if (LOG_Close() != LOG_OK) {
                    printf("LoggerTask: Failed to close file\r\n");
                }

                if (LOG_Unmount() != LOG_OK) {
                    printf("LoggerTask: Failed to unmount filesystem\r\n");
                }

                continue;
            }

            // 7. Isi record dari sensor
			DHT_Data data;
			float_to_string_dynamic(CurrentSensor_Read(), SensorData.current, 0);
			float_to_string_dynamic(VoltageSensor_Read(), SensorData.voltage, 0);

			if (DHT_Read(&dht, &data, false) == DHT_OK) {
				float_to_string_dynamic(data.temperature, SensorData.temp, 1);
				float_to_string_dynamic(data.humidity, SensorData.humidity, 0);

			} else {
				printf("Error baca DHT!\r\n");
			}

			snprintf(line, sizeof(line),
					 "%04d-%02d-%02dT%02d:%02d:%02d,%s,%s,%s,%s,%u",
					 now.date.year, now.date.month, now.date.day,
					 now.time.hours, now.time.minutes, now.time.seconds,
					 SensorData.temp, SensorData.humidity, SensorData.current, SensorData.voltage,
					 0xFFFFFFFF);
			//printf(line);

            // 8. Tulis record ke file
            if (LOG_Append(line) != LOG_OK) {
                printf("LoggerTask: Failed to write record\r\n");
                LOG_Close();
                LOG_Unmount();
            } else {
                LOG_Sync(); // flush ke SD card
                printf("Write Record.\r\n");
            }

     //       xSemaphoreGive(SD_Card_Mutex);
    //    }

        vTaskDelay(pdMS_TO_TICKS(TASK_LOGGER_INTERVAL )); // delay 5 detik
    }
}

void LOGGER_TaskCreate(SPI_Context *Storage_Ctx, I2C_RTCDevice *Rtc_Ctx, UBaseType_t priority) {
    LoggerParams *params = pvPortMalloc(sizeof(LoggerParams));
    params->storage = Storage_Ctx;
    params->rtc = Rtc_Ctx;
    xTaskCreate(vTaskLogger, "TaskLogger", 512, params, priority, NULL);
}
