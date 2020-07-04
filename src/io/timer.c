#include <avr/io.h>
#include "timer.h"

void setup1MSTimer(){
    TCCR0B |= _BV(CS01) | _BV(CS00);
    OCR0A = 250;
    TIMSK0 |= _BV(OCIE0A);
}