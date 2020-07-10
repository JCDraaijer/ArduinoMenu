#include "menu.h"
#include "io/iocontrol.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

int initialized = 0;
int running = 1;

uint8_t nextValue = 0x00;
uint8_t previousValue = 0x00;
uint16_t counter = 0;

// This interrupt is triggered every millisecond.
// It toggles PORTB (and the onboard-LED connected to it) and runs the menu
ISR(TIMER0_COMPA_vect) {
    TCNT0 = 0;
    if (PORTB != previousValue) {
        previousValue = PORTB;
        nextValue = ~previousValue;
        counter = 0;
    } else if (counter++ == 1000) {
        previousValue = PORTB;
        PORTB = nextValue;
        nextValue ^= 0xFFu;
        counter = 0;
    }
}

ISR(PCINT2_vect) {
    cli();
    // disableUartWakupPinInterrupt();
    setupUart();
    setup1MSTimer();
    printLine("Resuming execution...");
    running = 1;
    sei();
}

// Do initial setup, enable output on DDRB, setup the timer so it will trigger an interrupt and go to the vector
// defined above every millisecond
int main(void) {
    if (!initialized) {
        setupUart();
        DDRB = 0xff;
        setup1MSTimer();
        sei();
        initialized = 1;
    }
    MenuContext context = {};
    context.currentState = UNINITIALIZED;
    while (running) {
        showMenu(&context, &running);
    }
    /* cli();
    printLine("Going into sleep mode...");
    setupUartWakupPinInterrupt();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sei();
    sleep_mode();
    main();*/

}
