#include <util/atomic.h>

#include "project.h"
#include "timer.h"
#include "servo.h"

uint8_t servo_mode;
uint16_t servo_pulse;


//
// The SERVO (Regulator Control) signal of the alternator is an active low input
// intended to be driven by an open-collector type device.  If left unconnected
// the SERVO signal will assume a high level.  The SERVO signal is used to put the
// alternator into one of three modes of operation.  Here referred to as HIGH,
// LOW, and ACTIVE.
//
// When the SERVO signal is undriven or unconnected the alternator is in the HIGH
// mode.  In the HIGH mode the alternator is disabled, presenting only a small
// friction load to the engine.
//
// When the SERVO signal is driven or connected to ground the alternator is in the
// LOW mode.  In the LOW mode the alternator is enabled for operation at the
// default set point voltage of 14.03 volts.  In LOW mode the alternator will
// present a load to the engine proportional to the system electrical load.
//
// The alternator ACTIVE mode is selected when the SERVO signal is driven with a
// variable duty cycle, 125 Hz square wave.  In the ACTIVE mode the alternator
// is enabled for operation at a set point voltage determined by the duty cycle
// of the SERVO signal.  The legal duty cycle range is 5% to 95% corresponding to a
// set point voltage range of 12 to 16 volts.
//


// edge ISR variables
uint8_t new_servo_mode;
uint16_t new_servo_pulse;
uint16_t servo_cycle_start;


//
// the active high SERVO pulse in considered to have a start (the rising edge)
// and end (the falling edge) the start of the SERVO pulse always occurs at the
// start of the SERVO cycle
//
#define is_servo_cycle_start() test_bit(GPIOR0, 2)
#define set_servo_cycle_start() reg_set_bit(GPIOR0, 2)
#define clr_servo_cycle_start() reg_clear_bit(GPIOR0, 2)


void servo_init(void)
{
    // COM1A[1:0]=11b, set OC1A high on compare match
    TCCR1A |= _BV(COM1A1);
    TCCR1A |= _BV(COM1A0);

    // force compare match, set OC1A high
    TCCR1C |= _BV(FOC1A);

    // output high, pull-up on
    PORTB |= _BV(PORTB1);

    // output enable
    DDRB |= _BV(DDB1);

    // set the initial SERVO mode and count
    servo_mode = new_servo_mode = SERVO_MODE_OFF;
    servo_pulse = new_servo_pulse = SERVO_PULSE_LOW_LIMIT;
    set_servo_cycle_start();

    // set first cycle interrupt
    servo_cycle_start = TCNT1 + SERVO_PERIOD;
    OCR1A = servo_cycle_start;

    // clear timer compare A interrupt
    TIFR1 |= _BV(OCF1A);

    // enable timer compare A interrupt
    TIMSK1 |= _BV(OCIE1A);
}


void servo_set_mode(uint8_t mode, uint16_t pulse)
{
//    pulse =  (SERVO_PULSE_HIGH_LIMIT < pulse) ? SERVO_PULSE_HIGH_LIMIT :
//            ((SERVO_PULSE_LOW_LIMIT  > pulse) ? SERVO_PULSE_LOW_LIMIT  : pulse);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        new_servo_mode = mode;

        if (SERVO_MODE_ACTIVE == new_servo_mode) new_servo_pulse = pulse;
    }
}


ISR(TIMER1_COMPA_vect)
{
    if (is_servo_cycle_start())
    {
        if (SERVO_MODE_ACTIVE == servo_mode)
        {
            clr_servo_cycle_start();
            TCCR1A &= ~_BV(COM1A0);
            OCR1A = servo_cycle_start + servo_pulse;

            servo_cycle_start += SERVO_PERIOD;
            servo_mode = new_servo_mode;
            servo_pulse = new_servo_pulse;

            return;
        }
        else
        {
            servo_cycle_start += SERVO_PERIOD;
            servo_mode = new_servo_mode;
            servo_pulse = new_servo_pulse;
        }
    }
    else
        set_servo_cycle_start();

    if (SERVO_MODE_OFF == servo_mode)
        TCCR1A &= ~_BV(COM1A0);
    else
        TCCR1A |= _BV(COM1A0);

    OCR1A = servo_cycle_start;
}
