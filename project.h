#ifndef _PROJECT_H_
#define _PROJECT_H_

//
// avr-gcc c preprocessor search path
//
//  home/chris/bin/atmel/lib/gcc/avr/4.7.2/include
//  home/chris/bin/atmel/lib/gcc/avr/4.7.2/include-fixed
//  home/chris/bin/atmel/avr/include
//

#include <avr/io.h>
#include <stdint.h>

#define reg_clear_bit(a,b) do {(a) &= ~_BV(b);} while (0)
#define reg_set_bit(a,b) do {(a) |= _BV(b);} while (0)
#define reg_get_bit(a,b) ((a) & _BV(b))
#define test_bit(a,b) (((a) & _BV(b)) != 0)


#define GPFLAG0 (GPIOR0), (0)
#define GPFLAG1 (GPIOR0), (1)
#define GPFLAG2 (GPIOR0), (2)
#define GPFLAG3 (GPIOR0), (3)
#define GPFLAG4 (GPIOR0), (4)
#define GPFLAG5 (GPIOR0), (5)
#define GPFLAG6 (GPIOR0), (6)
#define GPFLAG7 (GPIOR0), (7)

#define TXIDLE 0
#define ECHO   1
#define ONLCR  2
#define ICRNL  3


static inline void set_gpflag(int flag)
{
    reg_set_bit(GPIOR0, flag);
}

static inline void clr_gpflag(int flag)
{
    reg_clear_bit(GPIOR0, flag);
}

static inline uint8_t get_gpflag(int flag)
{
    return reg_get_bit(GPIOR0, flag);
}

static inline uint8_t test_gpflag(int flag)
{
    return test_bit(GPIOR0, flag);
}


//
// timebase timer, 0, 1 or 2 (only 1 is extensively tested)
//
#define TBTIMER 1
#define TBTIMER_PRESCALER 64

//
// timebase counter size in bits, either 16 or 32
//
#define TBSIZE 32

#endif // _PROJECT_H_
