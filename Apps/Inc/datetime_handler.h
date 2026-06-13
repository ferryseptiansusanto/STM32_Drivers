/*
 * datetime_handler.h
 *
 *  Created on: 11 Jun 2026
 *      Author: ferry
 */

#ifndef INC_DATETIME_HANDLER_H_
#define INC_DATETIME_HANDLER_H_

#include "lcd_driver.h"
#include "command_event.h"
#include "ds3231_wrapper.h" // asumsi kamu punya driver RTC

void Datetime_HandleInput(I2C_LCDDevice *dev, CommandEvent *cmd);

#endif /* INC_DATETIME_HANDLER_H_ */
