/*
 * command_event.h
 *
 *  Created on: 19 May 2026
 *      Author: ferry
 */

#ifndef INC_TASKS_COMMAND_EVENT_H_
#define INC_TASKS_COMMAND_EVENT_H_


#include <stdbool.h>

typedef enum {
    CMD_KEYPAD,
    CMD_UART,
    CMD_SENSOR,
    CMD_SETTING
} CommandType;

typedef enum {
    KEY_NONE = 0,
    KEY_MENU,   // kombinasi * + #
    KEY_BACK,   // tombol *
    KEY_ENTER,  // tombol #
    KEY_UP,     // tombol A
    KEY_DOWN,   // tombol B
    KEY_LEFT,   // tombol C
    KEY_RIGHT   // tombol D
} KeypadKey;

typedef struct {
    CommandType type;
    union {
        struct {
            KeypadKey key;
            bool longPress;
        } keypad;
        struct {
            char buffer[32];
        } uart;
        struct {
            char value[32];
        } setting;
    } data;
} CommandEvent;

#endif /* INC_TASKS_COMMAND_EVENT_H_ */
