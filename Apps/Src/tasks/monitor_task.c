/*
 * my_monitor_task.c
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */

#include <rtc.h>
#include "monitor_task.h"
#include "logging.h"


void MonitorTask(void *argument) {
	RTC_TimeTypeDef time;

    // Jalankan device check sekali saat startup
	checkDevicesOnStartup();


    for (;;) {
        // Baca waktu RTC dengan proteksi mutex
		if (RTC_ReadTime(&time) == HAL_OK) {
			// Proteksi logger dengan mutex
			logInfo("MONITOR", "System running normally...");
			logInfo("TIME", "Now %02d:%02d:%02d %02d-%02d-%02d",
					time.hours, time.minutes, time.seconds,
					time.date, time.month, time.year);
		}

        vTaskDelay(pdMS_TO_TICKS(600000)); // delay 5 detik
    }
}


