#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include "iocontrol.h"

/* Enable UART
 * baud rate quickref:
 * BR    = UBRRval
 * 2400  = 416
 * 4800  = 207
 * 9600  = 103
 * 38.4k = 25
 * 250k  = 3
 */
void setupUart() {
    UBRR0 = 25;
    UCSR0B |= _BV(TXEN0) | _BV(RXEN0);
}

void disableUart() {
    UCSR0B &= ~(_BV(TXEN0) | _BV(RXEN0));
}

/*
 * Set the prescaler to 64
 * Set the comparison register to 250
 * 1/16000000 * (250 * 64) = 16000/16000000 = 1/1000 seconds -> 1000 Hz rate
 */
void setup1MSTimer() {
    TCCR0B |= _BV(CS01) | _BV(CS00);
    OCR0A = 250;
    TIMSK0 |= _BV(OCIE0A);
}

void disable1MSTimer() {
    if (TIMSK0 & _BV(OCIE0A)) {
        TIMSK0 ^= _BV(OCIE0A);
    }
}

void setupUartWakupPinInterrupt() {
    cli();
    disable1MSTimer();
    disableUart();
    PCMSK2 = _BV(PCINT16);
    PCICR = _BV(PCIE2);
    sei();
}

void disableUartWakupPinInterrupt() {
    PCMSK2 &= ~(_BV(PCINT16));
    PCICR &= ~(_BV(PCIE2));
    setupUart();
    setup1MSTimer();
}

// Convert a hex character to an integer (0-F and 0-f supported)
int8_t hexToInt(uint8_t character) {
    if (character >= '0' && character <= '9') {
        return (character - ASCII_DECIMAL_OFFSET);
    } else if (character >= 'A' && character <= 'F') {
        return (character - ASCII_HEX_UPPERCASE_OFFSET) + 10;
    } else if (character >= 'a' || character <= 'f') {
        return (character - ASCII_HEX_LOWERCASE_OFFSET) + 10;
    } else {
        return -1;
    }
}

// Convert an int to a hex character (0-16 supported)
int8_t intToHex(uint8_t integer) {
    if (integer >= 0 && integer <= 9) {
        return integer + ASCII_DECIMAL_OFFSET;
    } else if (integer >= 10 && integer <= 16) {
        return (integer - 10) + ASCII_HEX_UPPERCASE_OFFSET;
    } else {
        return -1;
    }
}

/*
 * Get a character from UART
 */
void getChar(CharResult *result, uint8_t newline, uint8_t blocking) {
    if (blocking) {
        while (!(UCSR0A & _BV(RXC0)));
    }
    if (!(UCSR0A & _BV(RXC0))) {
        result->success = 0;
    } else {
        result->success = 1;
        result->value = UDR0;
        printChar(result->value);
        if (newline) {
            printStr(NEWLINE);
        }
    }
}

// Send a character over UART (blocking)
void printChar(uint8_t data) {
    while (!(UCSR0A & _BV(UDRE0)));
    UDR0 = data;
}

void printStr(const char *data) {
    char value;
    while ((value = *(data++)) != 0) {
        printChar(value);
    }
}

void printHexInt(const uint8_t data) {
    printChar(intToHex((data & 0xF0) >> 4));
    printChar(intToHex(data & 0x0F));
}

void printLine(const char *data) {
    printStr(data);
    printStr(NEWLINE);
}

int twireadsingle(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer) {
    return twiread(slaveAddress, dataAddr, buffer, 1);
}

// An unsafe-as-all-hell twi read function (doesn't check for collisions or arbitration issues, just tries it's very best
// to read some data from the address that it's given
int twiread(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer, uint8_t len) {
    // Shift address to the left by one, so it will be put into the address register correctly
    slaveAddress = slaveAddress << 1;

    //TODO disable interrupts globally to prevent interruption of TWI operation?
    // Send the "start transmission" command
    TWBR = 9;
    TWCR = _BV(TWEN);
    TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));

    if (TW_STATUS != TW_START) {
        return -1;
    }

    // Send the address of the slave we wish to read from in order to write data address
    TWDR = slaveAddress | TW_WRITE;
    TWCR |= _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));
    if (TW_STATUS != TW_MT_SLA_ACK) {
        return -1;
    }

    // Send the data address we wish to read from
    TWDR = dataAddr;
    TWCR = _BV(TWEN) | _BV(TWINT);
    while (!(TWCR & _BV(TWINT)));
    if (TW_STATUS != TW_MT_DATA_ACK) {
        return -1;
    }

    // Resend the transmission start command
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
    while (!(TWCR & _BV(TWINT)));

    // Send the slave address and indicate that we wish to read from it
    TWDR = slaveAddress | TW_READ;
    TWCR = _BV(TWEN) | _BV(TWINT);

    while (!(TWCR & _BV(TWINT)));

    if (TW_STATUS != TW_MR_SLA_ACK) {
        return -1;
    }
    for (uint8_t twcr = _BV(TWEN) | _BV(TWINT) | _BV(TWEA); len > 0; len--) {
        if (len == 1) {
            twcr = _BV(TWEN) | _BV(TWINT);
        }
        TWCR = twcr;
        while (!(TWCR & _BV(TWINT)));

        uint8_t twstat = TW_STATUS;
        if ((len > 1 && twstat == TW_MR_DATA_ACK) || (len == 1 && twstat == TW_MR_DATA_NACK)) {
            *(buffer++) = TWDR;
        } else {
            return -1;
        }
    }
    return len;
}