#ifndef SPI_WRAPPER_H
#define SPI_WRAPPER_H

#include "stm32f1xx_hal.h"   // sesuaikan dengan seri MCU kamu
#include "FreeRTOS.h"
#include "semphr.h"

#define SPI_TIMEOUT_MS 1000

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

typedef enum {
    SPI_OK = 0,
    SPI_TIMEOUT,
    SPI_ERROR
} SPI_Status;

typedef enum {
    SPI_MODE_BLOCKING,
    SPI_MODE_DMA
} SPI_Mode;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;  // chip select port
    uint16_t cs_pin;        // chip select pin
    SPI_Mode mode;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
    SemaphoreHandle_t txrx_sem;
} SPI_Context;

extern SPI_Context spi1_ctx;
extern SPI_Context spi2_ctx;

// --- API Wrapper ---
void SPI_Init(SPI_Context *ctx);
SPI_Status SPI_Transmit(SPI_Context *ctx, const uint8_t *data, uint16_t size);
// ---Single byte---
SPI_Status SPI_Receive(SPI_Context *ctx, uint8_t *data, uint16_t size);
SPI_Status SPI_TransmitReceive(SPI_Context *ctx, uint8_t tx, uint8_t *rx );

SPI_Status SPI_TransmitBuffer(SPI_Context *ctx, const uint8_t *data, uint16_t size);
SPI_Status SPI_ReceiveBuffer(SPI_Context *ctx, uint8_t *data, uint16_t size);
SPI_Status SPI_TransmitReceiveBuffer(SPI_Context *ctx, const uint8_t *txBuf, uint8_t *rxBuf, uint16_t size);


void SPI_Select_CS(SPI_Context *ctx);
void SPI_Unselect_CS(SPI_Context *ctx);

void SPI_SetSpeed(SPI_Context *ctx, uint32_t prescaler);

#endif // SPI_WRAPPER_H
