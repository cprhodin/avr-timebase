#include <stdio.h>

#include "project.h"
#include "dds.h"


void dds_set(uint32_t tuning_word)
{
    dds_load(tuning_word, 0);
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

    // pulse DDS_W_CLK high
    PINB = DDS_W_CLK;
    PINB = DDS_W_CLK;

    // pulse DDS_FQ_UD high
    PINB = DDS_FQ_UD;
    PINB = DDS_FQ_UD;
}

void dds_init(void)
{
    // initialize the SPI pins
    PORTB &= ~DDS_PINS;
    DDRB  |= DDS_PINS;

    // initialize the SPI port
    spi_disable();
    SPSR = _BV(SPI2X);

    // reset DDS
    dds_reset();
}
