#include <avr/io.h>

#include "project.h"
#include "timer1.h"

#if   TBTIMER_PRESCALER == 1
#define TBTIMER_CS 1
#elif TBTIMER_PRESCALER == 8
#define TBTIMER_CS 2
#elif TBTIMER_PRESCALER == 64
#define TBTIMER_CS 3
#elif TBTIMER_PRESCALER == 256
#define TBTIMER_CS 4
#elif TBTIMER_PRESCALER == 1024
#define TBTIMER_CS 5
#else
#error "Invalid timer1 prescaler."
#endif

#if 0
#if   TBTIMER_PRESCALER == 1
#define TBTIMER_CS 1
#elif TBTIMER_PRESCALER == 8
#define TBTIMER_CS 2
#elif TBTIMER_PRESCALER == 32
#define TBTIMER_CS 3
#elif TBTIMER_PRESCALER == 64
#define TBTIMER_CS 3
#elif TBTIMER_PRESCALER == 128
#define TBTIMER_CS 3
#elif TBTIMER_PRESCALER == 256
#define TBTIMER_CS 4
#elif TBTIMER_PRESCALER == 1024
#define TBTIMER_CS 5
#else
#error "Invalid timer1 prescaler."
#endif
#endif

//
// timer 1 is configured in normal mode
//
//  the 16-bit counter operates continuously at clkIO/prescaler counting
//  from 0 to 65535 to 0, the rollover rate of the counter is
//
//    F_CPU / TBTIMER_PRESCALER / 65536
//    16 MHz / 64 / 65536 = 3.81 Hz
//
//  two output compare and one input capture unit are available
//
//  WGM1[3:0]=0000b, mode 0, normal, TOP = 0xffff
//  TBTIMER_CS[2:0]=xxxb, value determined by TBTIMER_PRESCALER
//
//  COM1A[1:0]=00b, normal port operation
//  COM1B[1:0]=00b, normal port operation
//  TIMSK1=0x00, all interrupts disabled
//  TCNT1=0x0000, start counter at 0
//
void timer1_init(void)
{
    // interrupts disabled
    TIMSK1 = 0x00;

    // disable asynchronous operation
    ASSR = 0x00;

    // COM1A[1:0]=00b, COM1B[1:0]=00b, WGM1[1:0]=00b
    TCCR1A = 0x00;

    // WGM1[3:2]=00b, TBTIMER_CS
    TCCR1B = 0x00 | TBTIMER_CS;

    // zero counter
    TCNT1 = 0x0000;
}
