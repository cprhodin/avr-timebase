#include <avr/io.h>
#include <stdio.h>

#include "project.h"

#include "dds.h"

#define DDS_FQ_UD _BV(PORTB2)
#define DDS_D7    _BV(PORTB3)
#define DDS_RESET _BV(PORTB4)
#define DDS_W_CLK _BV(PORTB5)
#define DDS_PINS (DDS_FQ_UD | DDS_D7 | DDS_RESET | DDS_W_CLK)

#define SPI_ENABLED  (           _BV(DORD) | _BV(MSTR))
#define SPI_DISABLED (_BV(SPE) | _BV(DORD) | _BV(MSTR))
#define spi_enable()  do {SPCR = SPI_ENABLED;} while (0)
#define spi_disable() do {SPCR = SPI_DISABLED;} while (0)


/*
  aa:   6e bd           out     0x2e, r22
  ac:   0d b4           in      r0, 0x2d
  ae:   07 fe           sbrs    r0, 7
  b0:   fd cf           rjmp    .-6
  aa:   6e bd           out     0x2e, r23
  ac:   0d b4           in      r0, 0x2d
  ae:   07 fe           sbrs    r0, 7
  b0:   fd cf           rjmp    .-6
  aa:   6e bd           out     0x2e, r24
  ac:   0d b4           in      r0, 0x2d
  ae:   07 fe           sbrs    r0, 7
  b0:   fd cf           rjmp    .-6
  aa:   6e bd           out     0x2e, r25
  ac:   0d b4           in      r0, 0x2d
  ae:   07 fe           sbrs    r0, 7
  b0:   fd cf           rjmp    .-6
  aa:   6e bd           out     0x2e, r1
  ac:   0d b4           in      r0, 0x2d
  ae:   07 fe           sbrs    r0, 7
  b0:   fd cf           rjmp    .-6
  e8:   84 e0           ldi     r22, 0x04
  ea:   83 b9           out     0x03, r22
  ec:   83 b9           out     0x03, r22
*/


void dds_set2(uint32_t delta)
{
    SPDR = (uint8_t) delta;
    while (!test_bit(SPSR, SPIF));

    SPDR = (uint8_t) (delta >> 8);

    while (!test_bit(SPSR, SPIF));

    SPDR = (uint8_t) (delta >> 16);
    while (!test_bit(SPSR, SPIF));

    SPDR = (uint8_t) (delta >> 24);
    while (!test_bit(SPSR, SPIF));

    SPDR = 0;
    while (!test_bit(SPSR, SPIF));

    // pulse FQ_UD high
    PINB = DDS_FQ_UD;
    PINB = DDS_FQ_UD;
}

void dds_set(uint32_t freq)
{
    unsigned long long tmp = (unsigned long long) freq << 32;
    uint32_t delta = (uint32_t) (tmp / F_DDS);
    uint8_t * p = (uint8_t *) &delta;

    // enable SPI
    spi_enable();

    SPDR = *p++;
    while (!test_bit(SPSR, SPIF));

    SPDR = *p++;
    while (!test_bit(SPSR, SPIF));

    SPDR = *p++;
    while (!test_bit(SPSR, SPIF));

    SPDR = *p++;
    while (!test_bit(SPSR, SPIF));

    SPDR = 0;
    while (!test_bit(SPSR, SPIF));

    // pulse FQ_UD high
    PINB = DDS_FQ_UD;
    PINB = DDS_FQ_UD;

    // disable SPI
    spi_disable();
}

void dds_power_down(void)
{
    // enable SPI
    spi_enable();

    // send power down command
    SPDR = 0x04;
    while (!test_bit(SPSR, SPIF));

    // pulse FQ_UD high
    PINB = DDS_FQ_UD;
    PINB = DDS_FQ_UD;

    // disable SPI
    spi_disable();
}

void dds_reset(void)
{
    // pulse DDS_RESET high
    PINB = DDS_RESET;
    PINB = DDS_RESET;
}

void dds_init(void)
{
    // initialize the SPI pins
    PORTB &= ~DDS_PINS;
    DDRB  |= DDS_PINS;

    // initialize the SPI port
    SPSR = SPI2X;
    spi_disable();

    // reset DDS
    dds_reset();
}
