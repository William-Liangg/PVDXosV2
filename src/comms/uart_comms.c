/**
 * @file uart_comms.c
 * @brief UART communications implementation for SERCOM0
 *
 * Implements interrupt-driven UART TX/RX with ring buffers.
 *
 * Created: 2026
 * Authors: PVDXos Team
 */

#include "uart_comms.h"
#include "ring_buffer.h"
#include "driver_init.h"
#include <atmel_start.h>
#include <hpl_usart_async.h>
#include <string.h>

/* Ring buffers for TX and RX */
static ring_buffer_t tx_ring_buffer;
static ring_buffer_t rx_ring_buffer;

/* Flag to track if TX is currently active */
static volatile bool tx_in_progress = false;

/* Forward declarations for internal functions */
static void uart_start_tx(void);

/**
 * @brief SERCOM0 DRE (Data Register Empty) interrupt handler
 *
 * Called when the USART data register is empty and ready for the next byte.
 * This is interrupt vector SERCOM0_0.
 */
void SERCOM0_0_Handler(void)
{
    uint8_t byte;

    /* Check if we have data to send */
    if (ring_buffer_read(&tx_ring_buffer, &byte) == 0) {
        /* Write the byte to the data register */
        _usart_async_write_byte(&USART_0, byte);
    } else {
        /* No more data - disable DRE interrupt */
        _usart_async_set_irq_state(&USART_0, USART_ASYNC_BYTE_SENT, false);
        tx_in_progress = false;
    }
}

/**
 * @brief SERCOM0 TXC (Transmit Complete) interrupt handler
 *
 * Called when transmission is fully complete.
 * This is interrupt vector SERCOM0_1.
 */
void SERCOM0_1_Handler(void)
{
    /* Clear the TXC interrupt flag by reading INTFLAG */
    ((Sercom *)SERCOM0)->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;
}

/**
 * @brief SERCOM0 RXC (Receive Complete) interrupt handler
 *
 * Called when a byte has been received.
 * This is interrupt vector SERCOM0_2.
 */
void SERCOM0_2_Handler(void)
{
    /* Read the received byte from the data register */
    uint8_t data = ((Sercom *)SERCOM0)->USART.DATA.reg;

    /* Store in the RX ring buffer (drop if full) */
    ring_buffer_write(&rx_ring_buffer, data);
}

/**
 * @brief SERCOM0 Error interrupt handler
 *
 * Called on framing error, parity error, or buffer overflow.
 * This is interrupt vector SERCOM0_3.
 */
void SERCOM0_3_Handler(void)
{
    /* Clear error flags by writing to INTFLAG */
    ((Sercom *)SERCOM0)->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_ERROR;

    /* Clear the status register errors */
    ((Sercom *)SERCOM0)->USART.STATUS.reg = SERCOM_USART_STATUS_PERR
                                          | SERCOM_USART_STATUS_FERR
                                          | SERCOM_USART_STATUS_BUFOVF;
}

/**
 * @brief Start TX transmission if not already in progress
 */
static void uart_start_tx(void)
{
    if (!tx_in_progress && !ring_buffer_is_empty(&tx_ring_buffer)) {
        tx_in_progress = true;
        /* Enable DRE interrupt to start transmission */
        _usart_async_set_irq_state(&USART_0, USART_ASYNC_BYTE_SENT, true);
    }
}

status_t uart_comms_init(void)
{
    /* Initialize ring buffers */
    ring_buffer_init(&tx_ring_buffer);
    ring_buffer_init(&rx_ring_buffer);

    /* Enable the USART peripheral */
    _usart_async_enable(&USART_0);

    /* Enable RX interrupt (always listening) */
    _usart_async_set_irq_state(&USART_0, USART_ASYNC_RX_DONE, true);

    /* Enable error interrupt */
    _usart_async_set_irq_state(&USART_0, USART_ASYNC_ERROR, true);

    return SUCCESS;
}

status_t uart_comms_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return SUCCESS;
    }

    /* Queue all bytes into TX ring buffer */
    for (uint16_t i = 0; i < len; i++) {
        if (ring_buffer_write(&tx_ring_buffer, data[i]) != 0) {
            /* Buffer full - start TX and return error */
            uart_start_tx();
            return ERROR_WRITE_FAILED;
        }
    }

    /* Start transmission */
    uart_start_tx();

    return SUCCESS;
}

uint16_t uart_comms_receive(uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }

    uint16_t count = 0;

    /* Read bytes from RX ring buffer */
    while (count < len) {
        if (ring_buffer_read(&rx_ring_buffer, &data[count]) != 0) {
            break;  /* No more data available */
        }
        count++;
    }

    return count;
}

size_t uart_comms_available(void)
{
    return ring_buffer_available(&rx_ring_buffer);
}

bool uart_comms_tx_complete(void)
{
    return !tx_in_progress && ring_buffer_is_empty(&tx_ring_buffer);
}

void uart_comms_flush_rx(void)
{
    ring_buffer_flush(&rx_ring_buffer);
}

status_t uart_comms_send_byte(uint8_t byte)
{
    /* Try to queue the byte, retrying if buffer is full */
    while (ring_buffer_write(&tx_ring_buffer, byte) != 0) {
        /* Buffer full - start TX and wait */
        uart_start_tx();
        /* Small delay to allow interrupt to process */
        for (volatile int i = 0; i < 100; i++);
    }

    uart_start_tx();
    return SUCCESS;
}

status_t uart_comms_send_string(const char *str)
{
    if (str == NULL) {
        return SUCCESS;
    }

    return uart_comms_send((const uint8_t *)str, strlen(str));
}
