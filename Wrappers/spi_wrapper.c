#include <spi_wrapper.h>
#include "delay.h"
#include <stdio.h>

SPI_Context spi1_ctx = { &hspi1, 0x00, 0x00, SPI_MODE_BLOCKING, NULL, NULL, NULL };
SPI_Context spi2_ctx = { &hspi2, 0x00, 0x00,SPI_MODE_BLOCKING, NULL, NULL, NULL};


void SPI_Init(SPI_Context *ctx) {
    // buat semaphore untuk sinkronisasi DMA
	ctx->tx_sem   = xSemaphoreCreateBinary();
	ctx->rx_sem   = xSemaphoreCreateBinary();
	ctx->txrx_sem = xSemaphoreCreateBinary();
    // cek alokasi
    if (!ctx->tx_sem || !ctx->rx_sem || !ctx->txrx_sem) {
        // gagal alokasi → bisa tambahkan error handler
    	printf("Gagal alokasi semaphore!\n");
    }
}

// --- Callback DMA ---
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // cari context yang sesuai
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (hspi == spi1_ctx.hspi) xSemaphoreGiveFromISR(spi1_ctx.tx_sem, &xHigherPriorityTaskWoken);
    else if (hspi == spi2_ctx.hspi) xSemaphoreGiveFromISR(spi2_ctx.tx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (hspi == spi1_ctx.hspi) xSemaphoreGiveFromISR(spi1_ctx.rx_sem, &xHigherPriorityTaskWoken);
    else if (hspi == spi2_ctx.hspi) xSemaphoreGiveFromISR(spi2_ctx.rx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (hspi == spi1_ctx.hspi) xSemaphoreGiveFromISR(spi1_ctx.txrx_sem, &xHigherPriorityTaskWoken);
    else if (hspi == spi2_ctx.hspi) xSemaphoreGiveFromISR(spi2_ctx.txrx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    printf("DMA TXRX complete ISR\r\n"); // debug log
}

// --- Transmit (1 byte) ---
SPI_Status SPI_Transmit(SPI_Context *ctx, const uint8_t *data, uint16_t size) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_Transmit_DMA(ctx->hspi, (uint8_t *)data, size) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->tx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS) return SPI_TIMEOUT;
    } else {
    	if (HAL_SPI_Transmit(ctx->hspi, (uint8_t *)data, size, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- Receive (1 byte) ---
SPI_Status SPI_Receive(SPI_Context *ctx, uint8_t *data, uint16_t size) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_Receive_DMA(ctx->hspi, data, size) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->rx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS) return SPI_TIMEOUT;
    } else {
    	if (HAL_SPI_Receive(ctx->hspi, data, size, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- TransmitReceive (1 byte) ---
SPI_Status SPI_TransmitReceive(SPI_Context *ctx, uint8_t tx, uint8_t *rx ) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_TransmitReceive_DMA(ctx->hspi, &tx, rx, 1) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->txrx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS)
        	{
        	printf("Timeout Waiting DMA\r\n"); // debug log
        		return SPI_TIMEOUT;
        	}

    } else {
    	if (HAL_SPI_TransmitReceive(ctx->hspi, &tx, rx, 1, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- Transmit (buffer, multi-byte) ---
SPI_Status SPI_TransmitBuffer(SPI_Context *ctx, const uint8_t *data, uint16_t size) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_Transmit_DMA(ctx->hspi, (uint8_t *)data, size) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->tx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS) return SPI_TIMEOUT;
    } else {
        if (HAL_SPI_Transmit(ctx->hspi, (uint8_t *)data, size, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- Receive (buffer, multi-byte) ---
SPI_Status SPI_ReceiveBuffer(SPI_Context *ctx, uint8_t *data, uint16_t size) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_Receive_DMA(ctx->hspi, data, size) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->rx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS) return SPI_TIMEOUT;
    } else {
        if (HAL_SPI_Receive(ctx->hspi, data, size, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- TransmitReceive (buffer, multi-byte) ---
SPI_Status SPI_TransmitReceiveBuffer(SPI_Context *ctx, const uint8_t *txBuf, uint8_t *rxBuf, uint16_t size) {
    if (ctx->mode == SPI_MODE_DMA) {
        if (HAL_SPI_TransmitReceive_DMA(ctx->hspi, (uint8_t *)txBuf, rxBuf, size) != HAL_OK) return SPI_ERROR;
        if (xSemaphoreTake(ctx->txrx_sem, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) != pdPASS) return SPI_TIMEOUT;
    } else {
        if (HAL_SPI_TransmitReceive(ctx->hspi, (uint8_t *)txBuf, rxBuf, size, SPI_TIMEOUT_MS) != HAL_OK) return SPI_ERROR;
    }
    return SPI_OK;
}

// --- CS Control ---
void SPI_Select_CS(SPI_Context *ctx) {
    HAL_GPIO_WritePin(ctx->cs_port, ctx->cs_pin, GPIO_PIN_RESET);
}

void SPI_Unselect_CS(SPI_Context *ctx) {
    HAL_GPIO_WritePin(ctx->cs_port, ctx->cs_pin, GPIO_PIN_SET);
}

// --- Speed Control ---
void SPI_SetSpeed(SPI_Context *ctx, uint32_t prescaler) {
    ctx->hspi->Init.BaudRatePrescaler = prescaler;
    HAL_SPI_Init(ctx->hspi);
    DelayMs(100);
}
