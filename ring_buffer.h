#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

struct ring_buffer {
    uint8_t * start;
    uint8_t * limit;
    uint8_t * volatile put;
    uint8_t * volatile get;
};


#define rb_is_empty(a) ((a)->get == (uint8_t *) (a))
#define rb_is_full(a) ((a)->put == (uint8_t *) (a))
#define rb_peek(a) (rb_is_empty(a) ? -1 : (int) *((a)->get))


void rb_init(struct ring_buffer * rb, uint8_t * p, size_t s);
int8_t rb_put(struct ring_buffer * rb, volatile uint8_t const * const b);
int8_t rb_get(struct ring_buffer * rb, volatile uint8_t * const b);

#endif // _RING_BUFFER_H_
