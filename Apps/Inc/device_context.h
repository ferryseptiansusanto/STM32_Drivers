/*
 * device_context.h
 *
 *  Created on: 6 May 2026
 *      Author: ferry
 */

#ifndef INC_DEVICE_CONTEXT_H_
#define INC_DEVICE_CONTEXT_H_

#include "spi_wrapper.h"
#include "i2c_wrapper.h"

typedef struct {
    I2C_Context *i2c;       // referensi ke i2c1_ctx / i2c2_ctx
    uint16_t address;  		// device address
    I2C_Mode mode;          // default mode device (DMA / blocking)
} I2CDeviceContext;


#endif /* INC_DEVICE_CONTEXT_H_ */
