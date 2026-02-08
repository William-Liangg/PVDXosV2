/**
 * @file uart_comms.h
 * @brief UART communications API layer for SERCOM0
 *
 * Provides buffered UART TX/RX using ring buffers and interrupt-driven I/O.
 *
 * Created: 2026
 * Authors: PVDXos Team
 */

#ifndef UART_COMMS_H
#define UART_COMMS_H

#include <stdint.h>
#include <stddef.h>
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the UART communications layer
 *
 * Sets up ring buffers, registers interrupt callbacks, and enables the USART.
 * Must be called after system_init() and before using other uart_comms functions.
 *
 * @return status_t SUCCESS on success, error code on failure
 */
status_t uart_comms_init(void);

/**
 * @brief Send data over UART
 *
 * Queues data into the TX ring buffer and starts transmission.
 * This function is non-blocking - it returns immediately after queuing.
 *
 * @param data Pointer to data buffer to send
 * @param len Number of bytes to send
 * @return status_t SUCCESS if all bytes queued, ERROR_WRITE_FAILED if buffer full
 */
status_t uart_comms_send(const uint8_t *data, uint16_t len);

/**
 * @brief Receive data from UART
 *
 * Reads available data from the RX ring buffer.
 * This function is non-blocking - returns immediately with available data.
 *
 * @param data Pointer to buffer to store received data
 * @param len Maximum number of bytes to read
 * @return uint16_t Number of bytes actually read (0 if none available)
 */
uint16_t uart_comms_receive(uint8_t *data, uint16_t len);

/**
 * @brief Get number of bytes available in RX buffer
 *
 * @return size_t Number of bytes waiting to be read
 */
size_t uart_comms_available(void);

/**
 * @brief Check if TX buffer is empty (all data sent)
 *
 * @return true if TX buffer is empty, false otherwise
 */
bool uart_comms_tx_complete(void);

/**
 * @brief Flush the RX buffer
 *
 * Discards all received data in the buffer.
 */
void uart_comms_flush_rx(void);

/**
 * @brief Send a single byte (blocking until space available)
 *
 * @param byte The byte to send
 * @return status_t SUCCESS on success
 */
status_t uart_comms_send_byte(uint8_t byte);

/**
 * @brief Send a null-terminated string
 *
 * @param str Pointer to null-terminated string
 * @return status_t SUCCESS if all bytes queued
 */
status_t uart_comms_send_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* UART_COMMS_H */
