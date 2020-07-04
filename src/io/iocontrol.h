#ifndef ARDUINOUNO_IOCONTROL_H
#define ARDUINOUNO_IOCONTROL_H

#include <stdint.h>

#define DS3231_ADDR 0x68

#define ASCII_DECIMAL_OFFSET 48
#define ASCII_HEX_OFFSET 65

#define NEWLINE "\r\n"

typedef struct CharResult {
    int8_t success;
    uint8_t value;
} CharResult;

void enableUart();

void setDDRB(uint8_t value);

void setPORTB(uint8_t value);

uint8_t getPORTB();

void getChar(CharResult *);

void sendChar(uint8_t data);

void sendLine(const char *data);

void sendStr(const char *data);

void sendHexInt(const uint8_t data);

void twireadsingle(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer);

void twiread(uint8_t slaveAddress, uint8_t dataAddr, uint8_t *buffer, uint8_t len);
#endif //ARDUINOUNO_IOCONTROL_H
