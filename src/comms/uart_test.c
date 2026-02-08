/**
 * @file uart_test.c
 * @brief Simple UART test implementation
 *
 * Created: 2026
 * Authors: PVDXos Team
 */

#include "uart_test.h"
#include "uart_comms.h"
#include <string.h>

/* Test message */
static const char greeting[] = "\r\n"
    "========================================\r\n"
    "       PVDXos UART Test Program\r\n"
    "========================================\r\n"
    "Hello from PVDX!\r\n"
    "UART configured: 115200 baud, 8N1\r\n"
    "Pins: PA04 (TX), PA05 (RX)\r\n"
    "Type something and it will be echoed back.\r\n"
    "----------------------------------------\r\n";

status_t uart_test_init(void)
{
    status_t status;

    /* Initialize the UART comms layer */
    status = uart_comms_init();
    if (status != SUCCESS) {
        return status;
    }

    /* Send greeting message */
    status = uart_comms_send((const uint8_t *)greeting, strlen(greeting));

    return status;
}

void uart_test_echo_once(void)
{
    uint8_t buf[32];
    uint16_t received;

    /* Check for received data */
    received = uart_comms_receive(buf, sizeof(buf));

    if (received > 0) {
        /* Echo back what we received */
        uart_comms_send(buf, received);

        /* Also send newline after carriage return for terminal compatibility */
        for (uint16_t i = 0; i < received; i++) {
            if (buf[i] == '\r') {
                uart_comms_send((const uint8_t *)"\n", 1);
            }
        }
    }
}

void uart_test_run(void)
{
    /* Initialize UART and send greeting */
    if (uart_test_init() != SUCCESS) {
        /* Initialization failed - nothing we can do without working UART */
        return;
    }

    /* Main echo loop - runs forever */
    while (1) {
        uart_test_echo_once();

        /* Small delay to avoid busy-waiting too hard */
        for (volatile uint32_t i = 0; i < 1000; i++);
    }
}
