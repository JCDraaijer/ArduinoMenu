#include <avr/io.h>
#include <util/twi.h>
#include "iocontrol.h"


void enableUart() {
    UBRR0 = 25;
    UCSR0B |= _BV(TXEN0) | _BV(RXEN0);
}

void getChar(CharResult *storage) {
    static char charSendBuffer[] = " ";
    if (!(UCSR0A & _BV(RXC0))) {
        storage->success = 0;
    } else {
        storage->success = 1;
        storage->value = UDR0;
        charSendBuffer[0] = storage->value;
        sendLine(charSendBuffer);
    }
}

void sendChar(const uint8_t data) {
    while (!(UCSR0A & _BV(UDRE0)));
    UDR0 = data;
}

void sendStr(const char *data){
    char value;
    while ((value = *(data++)) != 0) {
        sendChar(value);
    }
}

void sendLine(const char *data) {
    sendStr(data);
    sendChar('\r');
    sendChar('\n');
}

inline void setDDRB(uint8_t value) {
    DDRB = value;
}

inline uint8_t getPORTB() {
    uint8_t value = PORTB;
    return value;
}

inline void setPORTB(uint8_t value) {
    PORTB = value;
}

void twireadsingle(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer) {
    twiread(slaveAddress, dataAddr, buffer, 1);
}

void twiread(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer, uint8_t len) {
    // Shift address to the left by one, so it will be put into the address register correctly
    slaveAddress = slaveAddress << 1;

    //TODO disable interrupts globally to prevent interruption of TWI operation?

    // Send the "start transmission" command
    TWBR = 9;
    TWCR = _BV(TWEN);
    TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));

    // TODO check if arbitration and transmit were successful?

    // Send the address of the slave we wish to read from
    TWDR = slaveAddress | TW_WRITE;
    TWCR |= _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));

    // Send the data address we wish to read from
    TWDR = dataAddr;
    TWCR = _BV(TWEN) | _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));

    // Resend the transmission start command
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
    while (!(TWCR & _BV(TWINT)));

    // Send the slave address and indicate that we wish to read from it
    TWDR = slaveAddress | TW_READ;
    TWCR = _BV(TWEN) | _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));

    for (uint8_t twcr = _BV(TWEN) | _BV(TWINT) | _BV(TWEA); len > 0; len--) {
        if (len == 1) {
            twcr = _BV(TWEN) | _BV(TWINT);
        }
        TWCR = twcr;
        while (!(TWCR & _BV(TWINT)));
        *(buffer++) = TWDR;
    }
}