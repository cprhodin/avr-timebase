avr-timebase
============

Timer and timebase code for the Atmel ATmega328P.


build commands
==============

avr-gcc -Wall -std=c99 -mmcu=atmega328p -D__AVR_ATmega328P__ -DF_CPU=16000000 -c *.c

avr-gcc -Wall -std=c99 -mmcu=atmega328p -D__AVR_ATmega328P__ -DF_CPU=16000000 -o avr-timebase *.o
