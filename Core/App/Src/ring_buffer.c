/*
 * ring_buffer.c
 *
 *  Created on: Apr 1, 2021
 *      Author: karol
 */

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include "ring_buffer.h"
#include "main.h"

//--------------------------------------------------------------------------------

bool ring_buffer_init(struct ring_buffer *ring_buffer, uint8_t *data_buffer, size_t data_buffer_size)
{
    assert(ring_buffer);
    assert(data_buffer);
    assert(data_buffer_size > 0);

    if ((ring_buffer) && (data_buffer) && (data_buffer_size > 0))
    {
        ring_buffer->data_buffer = data_buffer;
        ring_buffer->data_buffer_size = data_buffer_size;
        ring_buffer->head = 0;
        ring_buffer->tail = 0;
        ring_buffer->count = 0;
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------

bool ring_buffer_clear(struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        for (size_t i = 0; i < ring_buffer->data_buffer_size; i++)
        {
            ring_buffer->data_buffer[i] = 0;
        }

        ring_buffer->count = 0;
        ring_buffer->head = 0;
        ring_buffer->tail = 0;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------

bool ring_buffer_is_empty(const struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if ((!ring_buffer->count) && (ring_buffer))
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------

size_t ring_buffer_get_len(const struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        return ring_buffer->count;
    }
    return 0;

}

//--------------------------------------------------------------------------------

size_t ring_buffer_get_capacity(const struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        return ring_buffer->data_buffer_size;
    }
    return 0;
}

//--------------------------------------------------------------------------------

bool ring_buffer_put_val(struct ring_buffer *ring_buffer, uint8_t val)
{
    assert(ring_buffer);

    if ((ring_buffer) && (ring_buffer->count != ring_buffer->data_buffer_size))
    {
        ring_buffer->data_buffer[ring_buffer->head] = val;

        ring_buffer->head++;
        ring_buffer->head %= ring_buffer->data_buffer_size;

        ring_buffer->count++;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------

bool ring_buffer_get_val(struct ring_buffer *ring_buffer, uint8_t *val)
{
    assert(ring_buffer);
    assert(val);

    if ((ring_buffer) && (val) && (ring_buffer->count))
    {
        *val = ring_buffer->data_buffer[ring_buffer->tail];

        ring_buffer->tail++;
        ring_buffer->tail %= ring_buffer->data_buffer_size;

        ring_buffer->count--;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------

bool ring_buffer_new_line_check(struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);


    size_t current_tail = ring_buffer->tail;
#ifdef WINDOWS
    size_t prev_tail = ring_buffer->tail + 1;
#endif

    for (uint8_t i = 0; i < ring_buffer->count; i++)
    {
#ifdef UNIX
        if(ring_buffer->data_buffer[current_tail] == '\r') return true;
        current_tail++;
#else
        if(ring_buffer->data_buffer[current_tail] == '\n' && ring_buffer->data_buffer[prev_tail] == '\r') return true;
        prev_tail = current_tail++;
#endif

        current_tail %= ring_buffer->data_buffer_size;
    }

    return false;
}

//--------------------------------------------------------------------------------

bool ring_buffer_esc_check(struct ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    size_t current_tail = ring_buffer->tail;

    for (uint8_t i = 0; i < ring_buffer->count; i++)
    {
        if(ring_buffer->data_buffer[current_tail++] == 27) return true;

        current_tail %= ring_buffer->data_buffer_size;
    }

    return false;
}

