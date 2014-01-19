#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#ifndef RING_BUFFER_ECHO
#define RING_BUFFER_ECHO 0
#endif

/*
 * ring-buffer control stucture
 */
struct ring_buffer {
    uint8_t * start;
    uint8_t * limit;
    uint8_t * volatile put;
    uint8_t * volatile get;

#if RING_BUFFER_ECHO != 0
    uint8_t * volatile echo;
#endif

    uint8_t flags;
};

/*
 * ring-buffer state flags
 */
#define RB_CANTPUT 0x01
#define RB_CANTGET 0x02

#if RING_BUFFER_ECHO != 0
#define RB_CANTECHO 0x04
#endif

/*
 * ring-buffer state tests
 */
#define rb_cantput(a) ((a)->flags & RB_CANTPUT)
#define rb_cantget(a) ((a)->flags & RB_CANTGET)
#define rb_full(a) rb_cantput(a)

#if RING_BUFFER_ECHO != 0
#define rb_cantecho(a) ((a)->flags & RB_CANTECHO)
#define rb_empty(a) (rb_cantecho(a) && rb_cantget(a))
#else
#define rb_empty(a) rb_cantget(a)
#endif

/*
 * clear ring-buffer
 */
#if RING_BUFFER_ECHO != 0
#define rb_clear(a) do {                                                       \
        (a)->flags = RB_CANTECHO | RB_CANTGET;                                 \
        (a)->get = (a)->echo = (a)->put = (a)->start;                          \
    } while (0)
#else
#define rb_clear(a) do {                                                       \
        (a)->flags = RB_CANTGET;                                               \
        (a)->get = (a)->put = (a)->start;                                      \
    } while (0)
#endif

/*
 * ring-buffer control/access functions
 */
extern void rb_init(struct ring_buffer * const rb, uint8_t * const p, size_t s);
extern int8_t rb_put(struct ring_buffer * rb, volatile uint8_t const * const b);
extern int8_t rb_unput(struct ring_buffer * rb, volatile uint8_t * const b);
extern int8_t rb_get(struct ring_buffer * rb, volatile uint8_t * const b);

#if RING_BUFFER_ECHO != 0
extern int8_t rb_put_echo(struct ring_buffer * rb,
                          volatile uint8_t const * const b);
extern int8_t rb_echo(struct ring_buffer * rb, volatile uint8_t * const b);
#endif

#endif // _RING_BUFFER_H_
