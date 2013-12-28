#include <stdio.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "project.h"
#include "ring_buffer.h"

#ifndef TX_BUF_SIZE
#define TX_BUF_SIZE (32)
#endif

#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE (32)
#endif

#ifndef CONSOLE_BAUDRATE
#define CONSOLE_BAUDRATE (250000L)
#endif

static uint8_t tx_buffer[TX_BUF_SIZE];
static uint8_t rx_buffer[RX_BUF_SIZE];
static struct ring_buffer tx_rb;
static struct ring_buffer rx_rb;


//
// USART interrupt handlers
//

// Tx complete
ISR(USART_TX_vect)
{
    UCSR0B &= ~_BV(TXCIE0);
    set_gpflag(TXIDLE);
}

// Tx data register empty
ISR(USART_UDRE_vect)
{
    // move byte from buffer to USART
    if (!rb_get(&tx_rb, &UDR0))
    {
        // disable interrupt if buffer becomes empty
        UCSR0B &= ~_BV(UDRIE0);
        UCSR0B |= _BV(TXCIE0);
    }
}

// Rx complete
ISR(USART_RX_vect)
{
    // move byte from USART to buffer
    if (!rb_put(&rx_rb, &UDR0)) {
        // disable interrupt if buffer becomes full
        UCSR0B &= ~_BV(RXCIE0);
    }
}


//
// putchar
//

static int console_putchar(char c, struct __file * stream)
{
    // handle NL to CR-NL expansion
    if (test_gpflag(ONLCR) && (c == '\n')) console_putchar('\r', stream);

    // wait for Tx buffer not full
    while (rb_is_full(&tx_rb));

    // queue byte
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        rb_put(&tx_rb, (uint8_t *) &c);
        UCSR0B |= _BV(UDRIE0);
        UCSR0B &= ~_BV(TXCIE0);
        clr_gpflag(TXIDLE);
    }

    return 0;
}


//
// getchar
//

static int console_getchar(struct __file * stream)
{
    uint8_t c;

#if 1 // O_NONBLOCK
    if (rb_is_empty(&rx_rb)) return _FDEV_EOF;
#else
    // wait for Rx buffer not empty
    while (rb_is_empty(&rx_rb));
#endif

    // dequeue byte
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        rb_get(&rx_rb, &c);
        UCSR0B |= _BV(RXCIE0);
    }

    // handle CR to NL translation
    if (test_gpflag(ICRNL) && (c == '\r')) c = '\n';

    // handle echo
    if (test_gpflag(ECHO)) console_putchar(c, stream);

    return c;
}


#define CONSOLE_UBRR ((F_CPU + (CONSOLE_BAUDRATE / 2)) /                       \
                      (CONSOLE_BAUDRATE * 16) - 1)                             \

static FILE console = FDEV_SETUP_STREAM(console_putchar, console_getchar,
                                        _FDEV_SETUP_RW);


//
// must be called with interrupts disabled
//
void console_init(void)
{
    // initialize the transmit and receive buffers
    rb_init(&tx_rb, tx_buffer, sizeof(tx_buffer));
    rb_init(&rx_rb, rx_buffer, sizeof(rx_buffer));

    // connect standard i/o to the serial console
    stdin = &console;
    stdout = &console;
    stderr = &console;

    // set the terminal attributes
    set_gpflag(ECHO); // enable input echo
    set_gpflag(ONLCR); // translate NL to CR-NL on output
    set_gpflag(ICRNL); // translate CR to NL on input

    // transmitter idle
    set_gpflag(TXIDLE);

    /* Set baud rate */
    UBRR0 = CONSOLE_UBRR;

    /* Enable receiver and transmitter */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    /* Set frame format: 8data, 2stop bit */
    UCSR0A = 0;
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

