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


void dds_set(uint32_t delta)
{
    // enable SPI
    spi_enable();

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
