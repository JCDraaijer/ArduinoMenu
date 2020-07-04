#include "menu.h"
#include "io/iocontrol.h"
#include "io/timer.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

int running = 1;
MenuState state = UNINITIALIZED;

uint8_t nextValue = 0x00;
uint8_t previousValue = 0x00;
uint16_t counter = 0;

ISR(TIMER0_COMPA_vect) {
    if (getPORTB() != previousValue) {
        previousValue = getPORTB();
        nextValue = ~previousValue;
        counter = 0;
    } else if (counter++ == 1000) {
        previousValue = getPORTB();
        setPORTB(nextValue);
        nextValue ^= 0xFFu;
        counter = 0;
    }
    if (running) {
        showMenu(&state, &running);
    }
    TCNT0 = 0;
}

int main(void) {
    enableUart();
    setDDRB(0xff);
    setup1MSTimer();
    sei();
    while (running);
    set_sleep_mode(SLEEP_MODE_IDLE);
}
