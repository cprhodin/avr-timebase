#include <stdio.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "project.h"
#include "timer.h"
#include "servo.h"
#include "dds.h"

#include "mathops.h"
#include "lerp.h"

extern void timer1_init(void);
extern void tick_init(void);
extern void console_init(void);

static struct interpolant percent_to_byte = {
    .x = 0,     // x0
    .dx = 100,  // x1 - x0
    .y = 0,     // y0
    .dy = 255,  // y1 - y0
};


static struct interpolant degree_to_servo = {
    .x = 0,
    .dx = 180,
    .y = 300,
    .dy = 300,
};


int main(void)
{
    uint16_t pulse_width = 250;

    uint16_t x;
    uint16_t y;
    uint16_t y0;

    // initialize
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        timer1_init();
        timebase_init();
        tick_init();
        servo_init();
        dds_init();
        console_init();
    }
    // interrupts are enabled

    printf("Up, up and away!\n");

//    servo_set_mode(SERVO_MODE_ACTIVE, SERVO_WIDTH_LOW_LIMIT);

    // use linear interpolation to translate 0-100% to 0-255
    y0 = percent_to_byte.y;
    for (x = 0; x <= 100; x++) {
        y = lerp(x, &percent_to_byte);
        printf("%3d : 0x%02x : %02d\n", x, y, y - y0);
        y0 = y;
    }

    for (;;) {
        char iobuffer[32];
        uint8_t n_io = 0;

        iobuffer[n_io] = '\0';

        printf("%d\n", pulse_width);

        servo_set_mode(SERVO_MODE_ACTIVE, pulse_width);

        for (;;) {
            int c;

            while ((c = getchar()) == EOF);

            if (c != '\n') {
                iobuffer[n_io++] = c;
                iobuffer[n_io] = '\0';

                if (n_io < 31) continue;
            }

            break;
        }

        printf("Input: %s\n", iobuffer);

        pulse_width = (uint16_t) strtol(iobuffer, NULL, 0);
    }


/*
    //        uint32_t frequency; // frequency * 32

    //        for (frequency = (440L << 5); frequency < (10000L << 5); frequency++) {
    //            dds_set(_ummd32(frequency, F_DDS << 5));

                timer_delay(TBTICKS_FROM_MS(2000));
                servo_set_mode(SERVO_MODE_ACTIVE, 250);

                timer_delay(TBTICKS_FROM_MS(2000));
                servo_set_mode(SERVO_MODE_ACTIVE, 300);
    //        }
*/

    return 0;
}
