#ifndef _PROJECT_H_
#define _PROJECT_H_

typedef signed char int8_t;
typedef signed int  int16_t;
typedef signed long int32_t;

typedef signed int  int_t;

typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;

typedef unsigned int  uint_t;


#define reg_clear_bit(a,b) do {(a) &= ~_BV(b);} while (0)
#define reg_set_bit(a,b) do {(a) |= _BV(b);} while (0)
#define reg_get_bit(a,b) ((a) & _BV(b))
#define test_bit(a,b) (((a) & _BV(b)) != 0)


#define EVENTS GPIOR0


#define GPFLAG0 100,0
#define GPFLAG1 (GPIOR0), (1)
#define GPFLAG2 (GPIOR0), (2)
#define GPFLAG3 (GPIOR0), (3)
#define GPFLAG4 (GPIOR0), (4)
#define GPFLAG5 (GPIOR0), (5)
#define GPFLAG6 (GPIOR0), (6)
#define GPFLAG7 (GPIOR0), (7)

//
// timebase timer, 0, 1 or 2 (only 1 is extensively tested)
//
#define TBTIMER 1

//
// timebase counter size in bits, either 16 or 32
//
#define TBSIZE 32

#endif // _PROJECT_H_
