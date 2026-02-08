/**
 * @file uart_test.h
 * @brief Simple UART test for verifying communication
 *
 * Created: 2026
 * Authors: PVDXos Team
 */

#ifndef UART_TEST_H
#define UART_TEST_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run the UART test
 *
 * Initializes UART, sends a greeting message, then enters an echo loop.
 * This function runs indefinitely - call it from main() for testing.
 *
 * Connect FTDI/USB-serial adapter:
 *   - PA04 (TX) -> FTDI RX
 *   - PA05 (RX) -> FTDI TX
 *   - GND -> GND
 *
 * Serial settings: 115200 baud, 8N1
 */
void uart_test_run(void);

/**
 * @brief Run a single iteration of the echo test
 *
 * Call this repeatedly from a task loop if you don't want blocking behavior.
 * Returns after processing any available RX data.
 */
void uart_test_echo_once(void);

/**
 * @brief Initialize UART and send greeting
 *
 * @return status_t SUCCESS on success
 */
status_t uart_test_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_TEST_H */
