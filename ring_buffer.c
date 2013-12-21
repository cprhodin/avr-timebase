#include <stddef.h>

#include "project.h"
#include "ring_buffer.h"


/*
 * simple byte at a time ring buffer
 *
 *  each ring buffer is defined by a structure with four elements
 *
 *   start - the start address of the buffer
 *   limit - the start address plus the buffer size, one past the end
 *   put   - the address to put the next byte into
 *   get   - the address to get the next byte from
 *
 *  when empty the get pointer is set to the address of the structure
 *
 *  when full the put pointer is set to the address of the structure
 */


void rb_init(struct ring_buffer * rb, uint8_t * p, size_t s)
{
    rb->start = p;
    rb->limit = p + s;
    rb->put = p;
    rb->get = (uint8_t *) rb; // empty
}


uint8_t rb_put(struct ring_buffer * rb, volatile uint8_t const * const b)
{
    uint8_t * put = rb->put;
    uint8_t * get = rb->get;

    /* return if buffer full */
    if (rb_is_full(rb))
        return 0;

    /* fix get pointer if buffer empty */
    if (rb_is_empty(rb))
        get = put;

    /* put byte into buffer */
    *put++ = *b;

    /* handle pointer wrap */
    if (put == rb->limit)
        put = rb->start;

    /* identify buffer full */
    if (put == get)
        put = (uint8_t *) rb;

    rb->put = put;
    rb->get = get;

    return 1;
}


uint8_t rb_get(struct ring_buffer * rb, volatile uint8_t * const b)
{
    uint8_t * put = rb->put;
    uint8_t * get = rb->get;

    /* return if buffer empty */
    if (rb_is_empty(rb))
        return 0;

    /* fix put pointer if buffer full */
    if (rb_is_full(rb))
        put = get;

    /* get byte from buffer */
    *b = *get++;

    /* handle pointer wrap */
    if (get == rb->limit)
        get = rb->start;

    /* identify buffer empty */
    if (get == put)
        get = (uint8_t *) rb;

    rb->put = put;
    rb->get = get;

    return 1;
}
