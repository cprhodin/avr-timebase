#ifndef _DDS_H_
#define _DDS_H_

//
// DDS module oscillator frequency
//
#define F_DDS (125000000L)

#define DDS_FQ_UD _BV(PORTB2)
#define DDS_D7 _BV(PORTB3)
#define DDS_RESET _BV(PORTB4)
#define DDS_W_CLK _BV(PORTB5)
#define DDS_PINS (DDS_FQ_UD | DDS_D7 | DDS_RESET | DDS_W_CLK)

#define SPI_DISABLED (           _BV(DORD) | _BV(MSTR))
#define SPI_ENABLED (_BV(SPE) | _BV(DORD) | _BV(MSTR))
#define spi_enable() do {SPCR = SPI_ENABLED;} while (0)
#define spi_disable() do {SPCR = SPI_DISABLED;} while (0)


static inline void dds_load(uint32_t tuning_word, uint8_t control)
{
    uint8_t ival;

    __asm__ __volatile__ (
        "               ldi             %[ival], %[spi_enabled]      \n"
        "               out             %[spcr], %[ival]             \n"
        "               out             %[spdr], %A[tuning_word]     \n"
        "               nop                                          \n"
        "1:             in              __tmp_reg__, %[spsr]         \n"
        "               sbrs            __tmp_reg__, %[spif]         \n"
        "               rjmp            1b                           \n"
        "               out             %[spdr], %B[tuning_word]     \n"
        "               nop                                          \n"
        "1:             in              __tmp_reg__, %[spsr]         \n"
        "               sbrs            __tmp_reg__, %[spif]         \n"
        "               rjmp            1b                           \n"
        "               out             %[spdr], %C[tuning_word]     \n"
        "               nop                                          \n"
        "1:             in              __tmp_reg__, %[spsr]         \n"
        "               sbrs            __tmp_reg__, %[spif]         \n"
        "               rjmp            1b                           \n"
        "               out             %[spdr], %D[tuning_word]     \n"
        "               nop                                          \n"
        "1:             in              __tmp_reg__, %[spsr]         \n"
        "               sbrs            __tmp_reg__, %[spif]         \n"
        "               rjmp            1b                           \n"
        "               out             %[spdr], %[control]          \n"
        "               nop                                          \n"
        "1:             in              __tmp_reg__, %[spsr]         \n"
        "               sbrs            __tmp_reg__, %[spif]         \n"
        "               rjmp            1b                           \n"
        "               ldi             %[ival], %[dds_fq_ud]        \n"
        "               out             %[pinb], %[ival]             \n"
        "               out             %[pinb], %[ival]             \n"
        "               ldi             %[ival], %[spi_disabled]     \n"
        "               out             %[spcr], %[ival]             \n"
        : [ival] "=&d" (ival)
        : [tuning_word] "r" (tuning_word),
          [control] "r" (control),
          // i/o registers
          [pinb] "I" _SFR_IO_ADDR(PINB),
          [spcr] "I" _SFR_IO_ADDR(SPCR),
          [spsr] "I" _SFR_IO_ADDR(SPSR),
          [spdr] "I" _SFR_IO_ADDR(SPDR),
          // i/o register bits
          [spi_enabled] "M" (SPI_ENABLED),
          [spi_disabled] "M" (SPI_DISABLED),
          [dds_fq_ud] "M" (DDS_FQ_UD),
          [spif] "M" (SPIF)
        : "r0"
    );
}

void dds_init(void);
void dds_reset(void);
void dds_power_down(void);
void dds_set(uint32_t tuning_word);

#endif // _DDS_H_
