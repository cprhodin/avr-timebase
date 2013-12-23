#include <avr/io.h>
#include <stdio.h>

#include "project.h"

#include "timer.h"


TIMER_EVENT(tick_timer_event, tick_timer_handler);
TIMER_EVENT(tick_off_event, tick_off_handler);


int8_t tick_timer_handler(struct timer_event * this_timer_event)
{
    // output low, ON
    PORTD &= ~_BV(PORTD4);

    // schedule the output off timer for 1 ms
    tick_off_event.tbtick = TBTICKS_FROM_MS(1);
    schedule_timer_event(&tick_off_event, this_timer_event);

    // advance this timer one second
    this_timer_event->tbtick += TBTICKS_FROM_MS(1000);

    // reschedule this timer
    return 1;
}


int8_t tick_off_handler(struct timer_event * this_timer_event)
{
    // output high, OFF
    PORTD |= _BV(PORTD4);

    // don't reschedule this timer
    return 0;
}


void tick_init(void)
{
    // initialize speaker output pin
    PORTD |= _BV(PORTD4);
    DDRD |= _BV(DDD4);

    // start tick timer
    tick_timer_event.tbtick = TBTICKS_FROM_MS(1000);
    schedule_timer_event(&tick_timer_event, NULL);
}
