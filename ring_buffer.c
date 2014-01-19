/*
 * simple byte at a time ring-buffer
 *
 *  each ring-buffer is defined by a structure with four elements
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


#include <stddef.h>

#include "project.h"
#include "ring_buffer.h"

/*
 * increment and wrap buffer pointer
 */
#define rb_inc_ptr(a,b) do {                                                   \
    b++;                                                                       \
    if (b == (a)->limit) b = (a)->start;                                       \
} while (0)

/*
 * (un)wrap and decrement buffer pointer
 */
#define rb_dec_ptr(a,b) do {                                                   \
    if (b == (a)->start) b = (a)->limit;                                       \
    b--;                                                                       \
} while (0)

/*
 * set ring-buffer state
 */
#define set_cantput(a) ((a)->flags |= RB_CANTPUT)
#define clr_cantput(a) ((a)->flags &= ~RB_CANTPUT)
#define set_cantget(a) ((a)->flags |= RB_CANTGET)
#define clr_cantget(a) ((a)->flags &= ~RB_CANTGET)

#if RING_BUFFER_ECHO != 0
#define set_cantecho(a) ((a)->flags |= RB_CANTECHO)
#define clr_cantecho(a) ((a)->flags &= ~RB_CANTECHO)
#endif

/*
 * initialize a ring-buffer control structure
 */
void rb_init(struct ring_buffer * const rb, uint8_t * const p, size_t s)
{
    rb->start = p;
    rb->limit = p + s;
    rb_clear(rb);
}


/*
 * rb_put - ring-buffer put
 *
 *  If space available, add byte to the tail of the ring-buffer and update the
 *  control structure.  This API also disables the echo facility.  Use this API
 *  when echo is not required.
 *
 * returns:  1 - byte added, more space available
 *           0 - byte added, no more space available
 *          -1 - byte not added, no space available
 */
int8_t rb_put(struct ring_buffer * const rb, volatile uint8_t const * const b)
{
    uint8_t * put;

    if (rb_cantput(rb)) return -1;

#if RING_BUFFER_ECHO != 0
    /* byte will not be available for echo */
    set_cantecho(rb);
#endif

    /* byte will be available for get */
    clr_cantget(rb);

    /* add byte, update pointers */
    put = rb->put;
    *put = *b;
    rb_inc_ptr(rb, put);
#if RING_BUFFER_ECHO != 0
    rb->echo = rb->put = put;
#else
    rb->put = put;
#endif

    if (put == rb->get) {
        /* no more space on exit */
        set_cantput(rb);
        return 0;
    }

    /* more space on exit */
    return 1;
}


/*
 * rb_unput - ring-buffer un-put
 *
 *  If available, return byte from the tail of the ring-buffer and update the
 *  control structure.
 *
 * returns:  1 - byte returned, more available
 *           0 - byte returned, no more available
 *          -1 - byte not returned, none available
 */
int8_t rb_unput(struct ring_buffer * const rb, volatile uint8_t * const b)
{
    uint8_t * put;

    if (rb_empty(rb)) return -1;

    /* space will be available for put */
    clr_cantput(rb);

    /* return byte, update pointer */
    put = rb->put;
    rb_dec_ptr(rb, put);
    *b = *put;
    rb->put = put;

#if RING_BUFFER_ECHO != 0
    if (rb_cantecho(rb)) {
        /* if can't echo on entry, can't echo on exit */
        rb->echo = put;
    }
    else if (rb->echo == put) {
        /* byte will not be available for echo */
        set_cantecho(rb);
    }
#endif

    if (rb->get == put) {
        set_cantget(rb);
        /* no more available on exit */
        return 0;
    }

    /* more available on exit */
    return 1;
}


/*
 * rb_get - ring-buffer get
 *
 *  If available, return byte from the head of the ring-buffer and update the
 *  control structure.
 *
 * returns:  1 - byte returned, more available
 *           0 - byte returned, no more available
 *          -1 - byte not returned, none available
 */
int8_t rb_get(struct ring_buffer * const rb, volatile uint8_t * const b)
{
    uint8_t * get;

    if (rb_cantget(rb)) return -1;

    /* space will be available for put */
    clr_cantput(rb);

    /* return byte, update pointer */
    get = rb->get;
    *b = *get;
    rb_inc_ptr(rb, get);
    rb->get = get;

#if RING_BUFFER_ECHO != 0
    if (get == rb->echo) {
#else
    if (get == rb->put) {
#endif
        /* no more available on exit */
        set_cantget(rb);
        return 0;
    }

    /* more available on exit */
    return 1;
}


#if RING_BUFFER_ECHO != 0
/*
 * rb_put_echo - ring-buffer put with echo
 *
 *  If space available, add byte to the tail of the ring-buffer and update the
 *  control structure.  This API also enables the echo facility.  Use this API
 *  when echo is required.
 *
 * returns:  1 - byte added, more space available
 *           0 - byte added, no more space available
 *          -1 - byte not added, no space available
 */

int8_t rb_put_echo(struct ring_buffer * rb, volatile uint8_t const * const b)
{
    uint8_t * put;

    if (rb_cantput(rb)) return -1;

    /* byte will be available for echo */
    clr_cantecho(rb);

    /* add byte, update pointer */
    put = rb->put;
    *put = *b;
    rb_inc_ptr(rb, put);
    rb->put = put;

    if (put == rb->get) {
        /* no more space on exit */
        set_cantput(rb);
        return 0;
    }

    /* more space on exit */
    return 1;
}


/*
 * rb_echo - ring-buffer echo
 *
 *  If available, return echo byte from the ring-buffer and update the control
 *  structure.
 *
 * returns:  1 - byte returned, more available
 *           0 - byte returned, no more available
 *          -1 - byte not returned, none available
 */
int8_t rb_echo(struct ring_buffer * const rb, volatile uint8_t * const b)
{
    uint8_t * echo;

    if (rb_cantecho(rb)) return -1;

    /* byte will be available for get */
    clr_cantget(rb);

    /* return byte, update pointer */
    echo = rb->echo;
    *b = *echo;
    rb_inc_ptr(rb, echo);
    rb->echo = echo;

    if (echo == rb->put) {
        /* echo byte not available on exit */
        set_cantecho(rb);
        return 0;
    }

    /* echo byte available on exit */
    return 1;
}
#endif
