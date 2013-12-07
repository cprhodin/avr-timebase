#include <util/atomic.h>

#include "project.h"
#include "timer1.h"
#include "timer.h"
#include "tick.h"

int main(void)
{
    // initialize
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        timer1_init();
        timebase_init();
        tick_init();
    }
    // interrupts are enabled

    for (;;);

    return 0;
}
