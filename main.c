#include <util/atomic.h>
#include <stdio.h>

#include "project.h"
#include "timer.h"
#include "dds.h"

#include "mathops.h"

extern void timer1_init(void);
extern void tick_init(void);
extern void uart_init(void);

//home/chris/bin/atmel/lib/gcc/avr/4.7.2/include
//home/chris/bin/atmel/lib/gcc/avr/4.7.2/include-fixed
//home/chris/bin/atmel/avr/include

int main(void)
{
    uint32_t frequency;

    // initialize
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        timer1_init();
        timebase_init();
        tick_init();
        dds_init();
        uart_init();
    }
    // interrupts are enabled

    printf("Up, up and away!\n");

    for (;;) {
        for (frequency = (440L << 5); frequency < (10000L << 5); frequency++) {
            dds_set(_ummd32(frequency, F_DDS << 5));
    /*
            timer_delay(TBTICKS_FROM_MS(250));

            dds_set(_ummd32(3000, F_DDS));
            timer_delay(TBTICKS_FROM_MS(500));

            dds_power_down();
            timer_delay(TBTICKS_FROM_MS(1000));

            dds_set(_ummd32(1000, F_DDS));
            timer_delay(TBTICKS_FROM_MS(250));
    */
        }
    }

    return 0;
}
