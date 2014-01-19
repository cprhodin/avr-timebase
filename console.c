#include <stdio.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sleep.h>

#include "project.h"
#include "ring_buffer.h"

/*
 * Special characters used for translation and processing.
 */
#define CR      ('\r')          // carriage return
#define ERASE   ('\b')          // backspace
#define KILL    ('U' & ~0x40)   // ctrl-U
#define NL      ('\n')          // new line (line feed)
#define SPACE   (' ')           // space

/*
 * Default terminal attributes.
 */
#ifndef is_icrnl
#define is_icrnl() (1)          // translate received CR to NL
#endif
#ifndef is_onlcr
#define is_onlcr() (1)          // translate transmitted NL to CR-NL
#endif
#ifndef is_echo
#define is_echo() (1)           // echo received characters to transmitter
#endif
#ifndef is_icanon
#define is_icanon() (1)         // enable canonical mode processing
#endif
#ifndef is_inonblock
#define is_inonblock() (0)      // block until operation is complete
#endif

/*
 * Transmit and receive ring-buffers.
 */
#ifndef TX_BUF_SIZE
#define TX_BUF_SIZE (32)
#endif
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE (32)
#endif
static uint8_t tx_buffer[TX_BUF_SIZE];
static uint8_t rx_buffer[RX_BUF_SIZE];
static struct ring_buffer tx_rb;
static struct ring_buffer rx_rb;

/*
 * Always points to the first character in the line or the next free space in
 * the buffer is the line is empty.
 */
static uint8_t * current_line;

/*
 * Variables used for I/O translation and processing.
 */
static uint8_t erase_count;
static uint8_t erase_state;
static uint8_t onlcr_state;


/*
 * Enable transmitter.
 */
#define tx_enable() do {                                                       \
        UCSR0B |= _BV(UDRIE0) | _BV(TXEN0);                                    \
        UCSR0B &= ~_BV(TXCIE0);                                                \
} while (0)

/*
 * Enable transmit complete interrupt.
 */
#define tx_complete() do {                                                     \
        UCSR0B |= _BV(TXCIE0) | _BV(TXEN0);                                    \
        UCSR0B &= ~_BV(UDRIE0);                                                \
} while (0)

/*
 * Disable transmitter.
 */
#define tx_disable() do {                                                      \
        UCSR0B &= ~(_BV(UDRIE0) | _BV(TXCIE0) | _BV(TXEN0));                   \
} while (0)


/*
 * Enable receiver.
 */
#define rx_enable() do {                                                       \
        UCSR0B |= _BV(RXEN0) | _BV(RXCIE0);                                    \
} while (0)

/*
 * Disable receiver (interrupt).
 */
#define rx_disable() do {                                                      \
        UCSR0B &= ~_BV(RXCIE0);                                                \
} while (0)


/*
 * Tx complete interrupt handler
 */
ISR(USART_TX_vect)
{
    tx_disable();
}


/*
 * Tx data register empty interrupt handler
 */
ISR(USART_UDRE_vect)
{
    uint8_t c;

    if (erase_count) {
        /*
         * (ECHOE) Echo error correcting ERASE.
         */
        switch (erase_state) {
        case 0:
            erase_state++;
            UDR0 = ERASE;
            return;
        case 1:
            erase_state++;
            UDR0 = SPACE;
            return;
        default:
            erase_count--;
            erase_state = 0;
            c = ERASE;
            break;
        }
    }
    else if (onlcr_state) {
        /*
         * (ONLCR) Complete NL to CR-NL expansion.
         */
        onlcr_state = 0;
        c = NL;
    }
    else {
        /*
         * Get next echo or output byte.
         */
        if (rb_echo(&rx_rb, &c) < 0) rb_get(&tx_rb, &c);

        if (is_onlcr() && (c == NL)) {
            /*
             * (ONLCR) Start NL to CR-NL expansion.
             */
            onlcr_state = 1;
            UDR0 = CR;
            return;
        }
    }

    /*
     * Put byte to USART.
     */
    UDR0 = c;

    if (rb_cantget(&tx_rb) && rb_cantecho(&rx_rb) &&
        !erase_count && !onlcr_state) {
        tx_complete();
    }
}


/*
 * Rx complete interrupt handler
 */
ISR(USART_RX_vect)
{
    uint8_t c;

    /*
     * Get byte from USART data register.
     */
    c = UDR0;

    /*
     * If enabled translate a received carriage-return (CR) to a newline (NL).
     */
    if (is_icrnl() && (c == CR)) c = NL;

    /*
     * Canonical Mode Input Processing
     *
     *  In canonical mode input processing, terminal input is processed in units
     *  of lines. A line is delimited by a newline character (NL). This means
     *  that a read request will not return until an entire line has been typed.
     *  Also, no matter how many bytes are requested in the read() call, at most
     *  one line will be returned. It is not, however, necessary to read a whole
     *  line at once; any number of bytes, even one, may be requested in a
     *  read() without losing information.
     *
     *  Erase and kill processing occur when either of two special characters,
     *  the ERASE and KILL characters, is received. This processing affects data
     *  in the input queue that has not yet been delimited by a newline (NL)
     *  character. This un-delimited data makes up the current line. The ERASE
     *  character deletes the last character in the current line, if there is
     *  one. The KILL character deletes all character in the current line, if
     *  there are any. The ERASE and KILL characters have no effect if there is
     *  no data in the current line. The ERASE and KILL characters themselves
     *  are not placed in the input queue.
     */
    if (is_icanon()) {
        switch (c) {
        case ERASE:
            /*
             * Handle the ERASE character if the current line is not empty.
             */
            if (rx_rb.put != current_line) {
                /*
                 * If the most recent character has been echoed, erase it.
                 */
                if (is_echo() && rb_cantecho(&rx_rb)) {
                    erase_count++;
                    tx_enable();
                }

                /*
                 * Remove the character from the input buffer.
                 */
                rb_unput(&rx_rb, &c);
            }

            /*
             * ERASE character is discarded.
             */
            return;
        case KILL:
            /*
             * Handle the KILL character if the current line is not empty.
             */
            while (rx_rb.put != current_line) {
                /*
                 * If the most recent character has been echoed, erase it.
                 */
                if (is_echo() && rb_cantecho(&rx_rb)) {
                    erase_count++;
                    tx_enable();
                }

                /*
                 * Remove the character from the input buffer.
                 */
                rb_unput(&rx_rb, &c);
            }

            /*
             * KILL character is discarded.
             */
            return;
        }
    }

    /*
     * Buffer incoming character.
     */
    if (!(is_echo() ? rb_put_echo(&rx_rb, &c) : rb_put(&rx_rb, &c)) &&
        is_icanon() && (c != NL)) {
        /*
         * in canonical mode the last character in the buffer must be a newline.
         */
        rb_unput(&rx_rb, &c);
    }
    else if (is_echo()) tx_enable();

    /*
     * A newline means the end of the current line and and the beginning of
     * a new current line.
     */
    if (c == NL) current_line = rx_rb.put;

    if (rb_full(&rx_rb)) rx_disable();
}


/*
 * putchar
 */
static int console_putchar(char c, struct __file * stream)
{
    set_sleep_mode(SLEEP_MODE_IDLE);
    for (;;) {
        cli();
        if (rb_put(&tx_rb, (uint8_t *) &c) >= 0) break;
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
    }

    tx_enable();
    sei();

    return 0;
}


/*
 * getchar
 */
static int console_getchar(struct __file * stream)
{
    char c;

    for (;;) {
        cli();
        /*
         * In canonical mode wait for the current line to be complete or the
         * buffer to be full, which is an error condition and should never
         * happen.
         */
        if (!is_icanon() || (rx_rb.get != current_line) || rb_full(&rx_rb))
            if (rb_get(&rx_rb, (uint8_t *) &c) >= 0) break;

        if (is_inonblock()) {
            sei();
            return _FDEV_EOF;
        }
        sei();
    }

    rx_enable();
    sei();

    return c;
}


static FILE console = FDEV_SETUP_STREAM(console_putchar, console_getchar,
                                        _FDEV_SETUP_RW);

/*
 * Initialize console interface. Must be called with interrupts disabled.
 */
void console_init(void)
{
    /*
     * Initialize the transmit and receive ring buffers.
     */
    rb_init(&tx_rb, tx_buffer, sizeof(tx_buffer));
    rb_init(&rx_rb, rx_buffer, sizeof(rx_buffer));

    /*
     * Current line is empty.
     */
    current_line = rx_rb.put;

    /*
     * State variables for processing.
     */
    erase_count = 0;
    erase_state = 0;
    onlcr_state = 0;

    /*
     * Attach the standard I/O to this console.
     */
    stdin = &console;
    stdout = &console;
    stderr = &console;

    /* Set baud rate */
    UBRR0 = UBRR_VALUE;

    /* Set frame format: 8data, 2stop bit */
    UCSR0A = 0;
    UCSR0B = 0;
    UCSR0C = CSTOPB | CS8;

    rx_enable();
}
