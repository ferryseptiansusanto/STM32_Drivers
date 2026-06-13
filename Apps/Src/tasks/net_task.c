/*
 * my_net_task.c
 *
 *  Created on: 12 Feb 2026
 *      Author: ferry
 */

#include "net_task.h"
#include "logging.h"

void NetTask(void *argument) {
    // Inisialisasi modul jaringan
    if (WrapperNet_Init() != WRAP_NET_OK) {
        logError("NET", "Failed to init network");
        vTaskDelete(NULL); // hentikan task jika gagal
    }

    for (;;) {
        // Polling loop untuk stack uIP
        WrapperNet_Poll();

        // Bisa tambahkan log debug periodik
        logDebug("NET", "Polling network stack...");

        // Delay agar tidak membebani CPU
        vTaskDelay(pdMS_TO_TICKS(100)); // setiap 100 ms
    }
}
