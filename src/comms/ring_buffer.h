/**
 * @file ring_buffer.h
 * @brief Lock-free ring buffer for single producer / single consumer use
 *
 * This ring buffer is designed for interrupt-safe operation where:
 * - The interrupt (producer) calls ring_buffer_write()
 * - The main code (consumer) calls ring_buffer_read()
 *
 * Thread safety is achieved through careful ordering of operations
 * and volatile head/tail pointers. No locks are required for SPSC use.
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default buffer size (must be power of 2 for efficient modulo) */
#ifndef RING_BUFFER_SIZE
#define RING_BUFFER_SIZE 64
#endif

/**
 * @brief Ring buffer structure
 *
 * head: index of next write position (modified by producer/interrupt)
 * tail: index of next read position (modified by consumer/main code)
 *
 * Buffer is empty when head == tail
 * Buffer is full when (head + 1) % size == tail
 * This wastes one slot but simplifies empty/full detection
 */
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile size_t head;  /**< Write index (producer modifies) */
    volatile size_t tail;  /**< Read index (consumer modifies) */
} ring_buffer_t;

/**
 * @brief Initialize a ring buffer
 * @param rb Pointer to ring buffer structure
 */
void ring_buffer_init(ring_buffer_t *rb);

/**
 * @brief Write a byte to the ring buffer
 * @param rb Pointer to ring buffer structure
 * @param byte The byte to write
 * @return 0 on success, -1 if buffer is full
 *
 * @note Safe to call from interrupt context (producer)
 */
int ring_buffer_write(ring_buffer_t *rb, uint8_t byte);

/**
 * @brief Read a byte from the ring buffer
 * @param rb Pointer to ring buffer structure
 * @param byte Pointer to store the read byte
 * @return 0 on success, -1 if buffer is empty
 *
 * @note Safe to call from main context (consumer)
 */
int ring_buffer_read(ring_buffer_t *rb, uint8_t *byte);

/**
 * @brief Check if the ring buffer is empty
 * @param rb Pointer to ring buffer structure
 * @return true if empty, false otherwise
 */
bool ring_buffer_is_empty(const ring_buffer_t *rb);

/**
 * @brief Check if the ring buffer is full
 * @param rb Pointer to ring buffer structure
 * @return true if full, false otherwise
 */
bool ring_buffer_is_full(const ring_buffer_t *rb);

/**
 * @brief Get the number of bytes available to read
 * @param rb Pointer to ring buffer structure
 * @return Number of bytes available in the buffer
 */
size_t ring_buffer_available(const ring_buffer_t *rb);

/**
 * @brief Get the number of free bytes available for writing
 * @param rb Pointer to ring buffer structure
 * @return Number of free bytes in the buffer
 */
size_t ring_buffer_free_space(const ring_buffer_t *rb);

/**
 * @brief Flush/clear the ring buffer
 * @param rb Pointer to ring buffer structure
 *
 * @warning Only call from consumer context when producer is not active
 */
void ring_buffer_flush(ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H */
