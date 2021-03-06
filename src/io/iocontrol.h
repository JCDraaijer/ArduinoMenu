#ifndef ARDUINOUNO_IOCONTROL_H
#define ARDUINOUNO_IOCONTROL_H

#include <stdint.h>

#define DS3231_ADDR 0x68

#define ASCII_DECIMAL_OFFSET 48
#define ASCII_HEX_UPPERCASE_OFFSET 65
#define ASCII_HEX_LOWERCASE_OFFSET 97

#define NEWLINE "\r\n"

typedef struct CharResult {
    int8_t success;
    uint8_t value;
} CharResult;

void setup1MSTimer();

void disable1MSTimer();

void setupUart();

void disableUart();

void setupUartWakupPinInterrupt();

void disableUartWakupPinInterrupt();

void getChar(CharResult *result, uint8_t newline, uint8_t blocking);

void printChar(uint8_t data);

void printLine(const char *data);

void printStr(const char *data);

void printHexInt(const uint8_t data);

int twireadsingle(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer);

int twiread(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer, uint8_t len);
#endif //ARDUINOUNO_IOCONTROL_H
