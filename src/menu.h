#ifndef ARDUINOUNO_MENU_H
#define ARDUINOUNO_MENU_H

#include <stdint.h>

typedef enum MenuState {
    UNINITIALIZED,
    MAIN_MENU,
    SECONDARY_MENU,
    TOGGLE_LED_MENU,
    I2C_MENU,
    I2C_MENU_SET_SLAVE,
    I2C_MENU_SET_ADDR,
    EXIT
} MenuState;

typedef struct SetInfo {
    uint8_t readFirstChar;
    uint8_t newValue;
} SetInfo;

typedef struct I2CInfo {
    uint8_t slaveAddress;
    uint8_t dataAddress;
} I2CInfo;

typedef struct MenuContext {
    MenuState currentState;
    SetInfo setInfo;
    I2CInfo i2CInfo;
    uint32_t callCounter;
    uint8_t printed;
} MenuContext;


void showMenu(MenuContext *info, int *continueRunning);

int8_t hexToInt(uint8_t character);

int8_t intToHex(uint8_t integer);

#endif //ARDUINOUNO_MENU_H
