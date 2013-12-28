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


void rb_init(struct ring_buffer * const rb, uint8_t * p, size_t s)
{
    rb->start = p;
    rb->limit = p + s;
    rb->put = p;
    rb->get = (uint8_t *) rb; // empty
}


/*
 * returns:
 *
 *   1 - byte queued, buffer not full
 *   0 - byte queued, buffer becomes full
 *  -1 - byte not queued, buffer full
 */
int8_t rb_put(struct ring_buffer * const rb, volatile uint8_t const * const b)
{
    uint8_t * put = rb->put;

    /* return if buffer full */
    if (put == (uint8_t *) rb)
        return -1;

    /* fix get pointer if buffer empty */
    if (rb->get == (uint8_t *) rb)
        rb->get = put;

    /* put byte into buffer */
    *put++ = *b;

    /* handle pointer wrap */
    if (put == rb->limit)
        put = rb->start;

    /* identify buffer becomes full */
    if (put == rb->get) {
        rb->put = (uint8_t *) rb;
        return 0;
    }

    rb->put = put;
    return 1;
}


/*
 * returns:
 *
 *   1 - byte dequeued, buffer not empty
 *   0 - byte dequeued, buffer becomes empty
 *  -1 - byte not dequeued, buffer empty
 */
int8_t rb_get(struct ring_buffer * const rb, volatile uint8_t * const b)
{
    uint8_t * get = rb->get;

    /* handle buffer empty */
    if (get == (uint8_t *) rb)
        return -1;

    /* fix put pointer if buffer full */
    if (rb->put == (uint8_t *) rb)
        rb->put = get;

    /* get byte from buffer */
    *b = *get++;

    /* handle pointer wrap */
    if (get == rb->limit)
        get = rb->start;

    /* handle buffer becomes empty */
    if (get == rb->put) {
        rb->get = (uint8_t *) rb;
        return 0;
    }

    rb->get = get;
    return 1;
}
