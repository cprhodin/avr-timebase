#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdio.h>

#include "project.h"
#include "ring_buffer.h"

uint8_t tx_buffer[32];
uint8_t rx_buffer[32];
struct ring_buffer tx_rb;
struct ring_buffer rx_rb;


//
// USART transmit
//

ISR(USART_TX_vect)
{
    UCSR0B &= ~_BV(TXCIE0);
    reg_clear_bit(GPIOR0, 1);
    PORTB &= ~0x1;
}

ISR(USART_UDRE_vect)
{
    // move a character from the buffer to the USART
    rb_get(&tx_rb, &UDR0);

    // disable the interrupt if the buffer is empty
    if (rb_is_empty(&tx_rb))
    {
        UCSR0B &= ~_BV(UDRIE0);
        UCSR0B |= _BV(TXCIE0);
    }
}

static int uart_putchar(char c, FILE * stream)
{
    if (c == '\n') uart_putchar('\r', stream);

    while (rb_is_full(&tx_rb));

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        rb_put(&tx_rb, (uint8_t *) &c);
        UCSR0B |= _BV(UDRIE0);
        UCSR0B &= ~_BV(TXCIE0);
        reg_set_bit(GPIOR0, 1);
        PORTB |= 0x1;
    }

    return 0;
}


//
// USART receive
//

ISR(USART_RX_vect)
{
    // move a character from the USART to the buffer
    rb_put(&rx_rb, &UDR0);

    // disable the interrupt if the buffer is full
    if (rb_is_full(&rx_rb)) UCSR0B &= ~_BV(RXCIE0);
}

static int uart_getchar(FILE * stream)
{
    char c;

#if 1
    if (rb_is_empty(&rx_rb)) return _FDEV_EOF;
#else
    while (rb_is_empty(&rx_rb));
#endif

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        rb_get(&rx_rb, (uint8_t *) &c);
        UCSR0B |= _BV(RXCIE0);
    }

    if (c == '\r') c = '\n';

    return c;
}


static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);


void uart_init(void)
{
    uint16_t ubrr = 3;

    DDRB |= 0x1;

    /* Set baud rate */
    UBRR0 = ubrr;

    /* Enable receiver and transmitter */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    /* Set frame format: 8data, 2stop bit */
    UCSR0A = 0;
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);

    stdin  = &mystdout;
    stdout = &mystdout;
    stderr = &mystdout;

    rb_init(&rx_rb, rx_buffer, sizeof(rx_buffer));
    rb_init(&tx_rb, tx_buffer, sizeof(tx_buffer));
}
