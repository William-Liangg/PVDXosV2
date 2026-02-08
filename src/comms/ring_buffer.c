/**
 * @file ring_buffer.c
 * @brief Lock-free ring buffer implementation for SPSC use
 */

#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

int ring_buffer_write(ring_buffer_t *rb, uint8_t byte)
{
    size_t head = rb->head;
    size_t next_head = (head + 1) % RING_BUFFER_SIZE;

    /* Check if buffer is full */
    if (next_head == rb->tail) {
        return -1;
    }

    rb->buffer[head] = byte;

    /*
     * Memory barrier would go here on platforms that need it.
     * ARM Cortex-M has strong ordering for normal memory accesses,
     * so a compiler barrier is sufficient.
     */
    __asm__ volatile("" ::: "memory");

    rb->head = next_head;

    return 0;
}

int ring_buffer_read(ring_buffer_t *rb, uint8_t *byte)
{
    size_t tail = rb->tail;

    /* Check if buffer is empty */
    if (tail == rb->head) {
        return -1;
    }

    *byte = rb->buffer[tail];

    /*
     * Memory barrier - ensure read completes before updating tail.
     * Compiler barrier is sufficient for Cortex-M.
     */
    __asm__ volatile("" ::: "memory");

    rb->tail = (tail + 1) % RING_BUFFER_SIZE;

    return 0;
}

bool ring_buffer_is_empty(const ring_buffer_t *rb)
{
    return rb->head == rb->tail;
}

bool ring_buffer_is_full(const ring_buffer_t *rb)
{
    return ((rb->head + 1) % RING_BUFFER_SIZE) == rb->tail;
}

size_t ring_buffer_available(const ring_buffer_t *rb)
{
    size_t head = rb->head;
    size_t tail = rb->tail;

    if (head >= tail) {
        return head - tail;
    } else {
        return RING_BUFFER_SIZE - tail + head;
    }
}

size_t ring_buffer_free_space(const ring_buffer_t *rb)
{
    /*
     * Max usable capacity is RING_BUFFER_SIZE - 1
     * (one slot is always kept empty to distinguish full from empty)
     */
    return (RING_BUFFER_SIZE - 1) - ring_buffer_available(rb);
}

void ring_buffer_flush(ring_buffer_t *rb)
{
    rb->tail = rb->head;
}
