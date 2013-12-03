#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdio.h>
#include <ctype.h>

#include "project.h"
#include "timer1.h"

int main(void)
{
    // initialize
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        timer1_init();
    }
    // interrupts are enabled

    for (;;);

    return 0;
}
