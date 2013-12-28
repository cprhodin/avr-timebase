#ifndef _SERVO_H_
#define _SERVO_H_

// operating modes
#define SERVO_MODE_OFF 0
#define SERVO_MODE_ACTIVE 1

// servo timing
#define SERVO_FREQ 100L

#define SERVO_PERIOD (F_CPU / (SERVO_FREQ * TBTIMER_PRESCALER))

#define SERVO_PULSE_LOW_LIMIT (SERVO_PERIOD * 5L / 100L) // 5%
#define SERVO_PULSE_HIGH_LIMIT (SERVO_PERIOD - SERVO_PULSE_LOW_LIMIT) // 95%

#define SERVO_COUNTS_PER_MS (F_CPU / (TBTIMER_PRESCALER * 1000L))

void servo_init(void);
void servo_set_mode(uint8_t mode, uint16_t pulse);

#endif // _SERVO_H_
