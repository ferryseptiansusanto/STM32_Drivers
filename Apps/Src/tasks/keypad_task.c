/*
 * keypad_task.c
 *
 *  Created on: 8 May 2026
 *      Author: ferry
 */
#include "keypad_task.h"
#include "command_task.h"
#include "delay.h"
#include "keypad_4x4.h"
#include "command_event.h"
#include <stdio.h>

extern QueueHandle_t commandQueue;
extern I2C_Context i2c1_ctx;

#define DEBOUNCE_COUNT   3
#define LONGPRESS_MS     2000

KeypadKey MapKeypadChar(char rawKey, bool longPress) {
    switch (rawKey) {
        case '*':
            // Long press '#' → masuk menu
            return longPress ? KEY_MENU : KEY_BACK;
        case '#':
            return KEY_ENTER;   // atau bisa dipakai untuk fungsi lain
        case 'A':
            return KEY_UP;
        case 'B':
            return KEY_DOWN;
        case 'C':
            return KEY_LEFT;
        case 'D':
            return KEY_RIGHT;
        default:
            return (KeypadKey)rawKey; // fallback: kirim char mentah
    }
}

static void vTaskKeypad(void *pvParameters) {
    I2C_KeypadDevice *dev = (I2C_KeypadDevice*)pvParameters;

    // Variabel state-machine penahan kondisi tombol
    char rawSampleKey = 0;       // Untuk memfilter bounce mekanis
    char lastStableKey = 0;      // Menyimpan status tombol stabil sebelumnya

    TickType_t pressStart = 0;
    int debounceCounter = 0;
    bool isLongPressedTriggered = false;

    KEYPAD_InitCmd(dev);

    while (1) {
        char currentKey = KEYPAD_GetKey(dev);

        // --- 1. Logika Debounce yang Valid ---
        // Membandingkan input saat ini dengan sampel mentah sebelumnya (bukan tombol aktif)
        if (currentKey == rawSampleKey) {
            if (debounceCounter < DEBOUNCE_COUNT) {
                debounceCounter++;
            }
        } else {
            rawSampleKey = currentKey;
            debounceCounter = 0;
        }

        // --- 2. Proses Logika Tombol Setelah Lolos Debounce ---
        if (debounceCounter >= DEBOUNCE_COUNT) {

            // KONDISI A: Tombol Baru Saja Ditekan (Transisi dari IDLE ke PRESSED)
            if (rawSampleKey != 0 && lastStableKey == 0) {
                lastStableKey = rawSampleKey;
                pressStart = xTaskGetTickCount();
                isLongPressedTriggered = false;
            }

            // KONDISI B: Tombol Masih Ditahan (STILL PRESSED) -> Proses Long Press & Kombinasi Menu
            else if (rawSampleKey != 0 && lastStableKey == rawSampleKey) {

                // Cek Fitur Spesial Kombinasi Menu (* + #)
                // Membaca kombinasi dari status hardware mentah saat ini
                if (rawSampleKey == '*' || rawSampleKey == '#') {
                    // Beberapa library mengembalikan karakter khusus jika dual-press terdeteksi,
                    // atau Anda bisa modifikasi fungsi KEYPAD_GetKey untuk membaca multi-register.
                    // Jika library Anda hanya mengembalikan 1 karakter dominan, penanganan di bawah ini mengamankannya.
                }

                // Logika Evaluasi Waktu Long Press
                if (!isLongPressedTriggered) {
                    if ((xTaskGetTickCount() - pressStart) > pdMS_TO_TICKS(LONGPRESS_MS)) {
                        isLongPressedTriggered = true;

                        CommandEvent cmd;
                        cmd.type = CMD_KEYPAD;
                        //cmd.data.keypad.key = lastStableKey;
                        cmd.data.keypad.longPress = true;
                        cmd.data.keypad.key = MapKeypadChar(lastStableKey, cmd.data.keypad.longPress);

                         xQueueSend(commandQueue, &cmd, 0);

                        printf("[KEYPAD] Long press: %c\r\n", lastStableKey);
                    }
                }
            }

            // KONDISI C: Tombol Baru Saja Dilepas (Transisi dari PRESSED ke IDLE) -> Proses Short Press
            else if (rawSampleKey == 0 && lastStableKey != 0) {

                // Hanya kirim event Short Press jika tombol tersebut BELUM memicu Long Press
                if (!isLongPressedTriggered) {
                    CommandEvent cmd;
//                    cmd.data.keypad.key = lastStableKey;
                    cmd.data.keypad.longPress = false;
                    cmd.type = CMD_KEYPAD;
                    cmd.data.keypad.key = MapKeypadChar(lastStableKey, cmd.data.keypad.longPress);


                    xQueueSend(commandQueue, &cmd, 0);

                    printf("[KEYPAD] Short press: %c\r\n", lastStableKey);
                }

                // Reset penanda state untuk tombol berikutnya
                lastStableKey = 0;
                isLongPressedTriggered = false;
            }

            // KONDISI D: Pengguna geser jari dari tombol A ke tombol B tanpa lepas tengah
            else if (rawSampleKey != 0 && lastStableKey != rawSampleKey) {
                lastStableKey = rawSampleKey;
                pressStart = xTaskGetTickCount();
                isLongPressedTriggered = false;
            }
        }

        // Menggunakan DelayMs bungkus vTaskDelay Anda.
        // Diturunkan ke 20ms agar total respons filter instan (3 * 20ms = 60ms)
        DelayMs(20);
    }
}

void KEYPAD_TaskCreate(I2C_KeypadDevice *dev, UBaseType_t priority) {
    xTaskCreate(vTaskKeypad, "KeypadTask", 256, dev, priority, NULL);
}

