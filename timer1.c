#include <avr/io.h>

#include "project.h"
#include "timer1.h"

//
// timer 1 is configured in normal mode
//
//  the 16-bit counter operates continuously at clkIO/64 counting from 0 to
//  65535 to 0, the rollover rate of the counter is 3.81 Hz with a 16 MHz
//  crystal (16 MHz / 64 / 65536)
//
//  two output compare and one input capture unit are available
//
//  WGM1[3:0]=0000b, mode 0, normal, TOP = 0xffff
//  CS1[2:0]=011b, clock select clkIO/64
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

    // WGM1[3:2]=00b, CS1[2:0]=011b
    TCCR1B = 0x03;

    // zero counter
    TCNT1 = 0x0000;
}
