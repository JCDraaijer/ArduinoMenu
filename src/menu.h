#ifndef ARDUINOUNO_MENU_H
#define ARDUINOUNO_MENU_H

#include <stdint.h>

#define ASCII_DECIMAL_OFFSET 48
#define ASCII_HEX_OFFSET 65

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

void showMenu(MenuState *state, int *continueRunning);

int8_t hexToInt(uint8_t character);

int8_t intToHex(uint8_t integer);

#endif //ARDUINOUNO_MENU_H
