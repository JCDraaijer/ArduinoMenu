#include "menu.h"
#include "io/iocontrol.h"

static uint8_t i2cslaveAddress = 0x68;
static uint8_t i2cdataAddress = 0x11;

MenuState showMainMenu(int *printed);

MenuState showSecondaryMenu(int *printed);

MenuState showLedMenu(int *printed);

MenuState showI2CMenu(int *printed);

MenuState showI2CSetMenu(int *printed, uint8_t *valueToSet, MenuState originalState);

void printMenu(const char *const *strings, int amtOfStrings, int *printed, CharResult *inputResult);

void printTemperature();

void showMenu(MenuState *state, int *continueRunning) {
    static uint32_t counter = 0;
    counter++;
    MenuState currentState = *state;
    static int printed = 0;

    switch (currentState) {
        case UNINITIALIZED:
        case MAIN_MENU:
            *state = showMainMenu(&printed);
            break;
        case SECONDARY_MENU:
            *state = showSecondaryMenu(&printed);
            break;
        case TOGGLE_LED_MENU:
            *state = showLedMenu(&printed);
            break;
        case I2C_MENU:
            *state = showI2CMenu(&printed);
            break;
        case I2C_MENU_SET_ADDR:
            *state = showI2CSetMenu(&printed, &i2cdataAddress, *state);
            break;
        case I2C_MENU_SET_SLAVE:
            *state = showI2CSetMenu(&printed, &i2cslaveAddress, *state);
            break;
        case EXIT:
        default:
            sendLine("Exiting program...");
            *continueRunning = 0;
            break;
    }
}


int8_t hexToInt(uint8_t character) {
    if (character >= '0' && character <= '9') {
        return (character - ASCII_DECIMAL_OFFSET);
    } else if (character >= 'A' && character <= 'F') {
        return (character - ASCII_HEX_OFFSET) + 10;
    } else {
        return -1;
    }
}

int8_t intToHex(uint8_t integer) {
    if (integer >= 0 && integer <= 9) {
        return integer + ASCII_DECIMAL_OFFSET;
    } else if (integer >= 10 && integer <= 16) {
        return (integer - 10) + ASCII_HEX_OFFSET;
    } else {
        return -1;
    }
}

void sendUnknownCharMsg(char character) {
    static char *data = "Unknown option \" \"";
    data[16] = character;
    sendLine(data);
}

void printMenu(const char *const *strings, const int amtOfStrings, int *printed, CharResult *inputResult) {
    if (!*printed) {
        for (int i = 0; i < amtOfStrings; i++) {
            sendLine(strings[i]);
        }
        *printed = 1;
    }

    getChar(inputResult);
    if (inputResult->success) {
        *printed = 0;
    }
}


MenuState showMainMenu(int *printed) {
    const char *const menuitems[] =
            {"Main menu",
             "Options:",
             "a - print some other text",
             "b - go to the 2nd menu",
             "c - reprint the main menu",
             "l - go to the LED menu",
             "p - print seconds thingie",
             "r - go to i2c menu",
             "q - exit the program"};

    CharResult input;
    printMenu(menuitems, sizeof(menuitems) / sizeof(char **), printed, &input);

    if (!input.success) {
        return MAIN_MENU;
    }

    switch (input.value) {
        case 'a':
            sendLine("Here we print something");
            return MAIN_MENU;
        case 'b':
            return SECONDARY_MENU;
        case 'c':
            return MAIN_MENU;
        case 'q':
            return EXIT;
        case 'l':
            return TOGGLE_LED_MENU;
        case 'p':
            printTemperature();
            return MAIN_MENU;
        case 'r':
            return I2C_MENU;
        default: {
            sendUnknownCharMsg(input.value);
            return MAIN_MENU;
        }
    }
}

MenuState showI2CMenu(int *printed) {
    const char *const menuItems[] = {
            "I2C Menu:",
            "Options:",
            "s - set slave address",
            "d - set data address",
            "r - read a byte",
    };
    CharResult input;
    if (!*printed) {
        sendStr("SLV addr: ");
        sendChar(intToHex((i2cslaveAddress & 0xF0) >> 4));
        sendChar(intToHex(i2cslaveAddress & 0x0F));
        sendStr("\r\n");
        sendStr("DATA addr: ");
        sendChar(intToHex((i2cdataAddress & 0xF0) >> 4));
        sendChar(intToHex(i2cdataAddress & 0x0F));
        sendStr("\r\n");
    }
    printMenu(menuItems, sizeof(menuItems) / sizeof(char **), printed, &input);
    if (!input.success) {
        return I2C_MENU;
    }
    switch (input.value) {
        case 's':
            return I2C_MENU_SET_SLAVE;
        case 'd':
            return I2C_MENU_SET_ADDR;
        case 'r': {
            uint8_t value;
            twireadsingle(i2cslaveAddress, i2cdataAddress, &value);
            sendStr("DATA value: 0x");
            sendChar(intToHex((value & 0xF0) >> 4));
            sendChar(intToHex(value & 0x0F));
            sendStr("\r\n");
            return I2C_MENU;
        }
        case 'q':
            return MAIN_MENU;
        default:
            sendUnknownCharMsg(input.value);
            return I2C_MENU;
    }
}

MenuState showI2CSetMenu(int *printed, uint8_t *toSet, MenuState originalState) {
    static uint8_t readFirstChar = 0;
    static uint8_t newValue;

    const char *const menuItemsFirst[] = {
            "Input most significant hex digit (or q):",
            "Input least significant hex digit (or q):"
    };

    if (!readFirstChar) {
        CharResult input;
        printMenu(menuItemsFirst, 1, printed, &input);
        if (!input.success) {
            return originalState;
        }

        int8_t value = hexToInt(input.value);

        if (value != -1) {
            newValue &= 0x0F;
            newValue += value << 4;
        } else if (input.value == 'q') {
            readFirstChar = 0;
            return I2C_MENU;
        } else {
            sendLine("Invalid input. (Must be 0-F");
            return originalState;
        }
        readFirstChar = 1;
        return originalState;
    } else {
        CharResult input;
        printMenu(menuItemsFirst + 1, 1, printed, &input);
        if (!input.success) {
            return originalState;
        }

        int8_t value = hexToInt(input.value);

        if (value != -1) {
            newValue &= 0xF0;
            newValue += value;
        } else if (input.value == 'q') {
            readFirstChar = 0;
            return I2C_MENU;
        } else {
            sendLine("Invalid input. (Must be 0-F)");
            return originalState;
        }
        readFirstChar = 0;
        *toSet = newValue;
        return I2C_MENU;
    }
}

MenuState showSecondaryMenu(int *printed) {
    const char *const menuItems[] = {
            "Secondary menu",
            "Options:",
            "a - reprint this menu",
            "q - return to the main menu"
    };
    CharResult input;
    printMenu(menuItems, sizeof(menuItems) / sizeof(char **), printed, &input);
    if (!input.success) {
        return SECONDARY_MENU;
    }

    switch (input.value) {
        case 'a':
            return SECONDARY_MENU;
        case 'q':
            return MAIN_MENU;
        default:
            sendUnknownCharMsg(input.value);
            return SECONDARY_MENU;
    }
}

MenuState showLedMenu(int *printed) {
    const char *const menuItems[] = {
            "Led toggle menu",
            "Options:",
            "a - toggle LED",
            "b - turn LED on",
            "c - turn LED off",
            "q - return to the main menu"
    };

    CharResult input;
    printMenu(menuItems, sizeof(menuItems) / sizeof(char **), printed, &input);

    if (!input.success) {
        return TOGGLE_LED_MENU;
    }

    uint8_t portBvalue = getPORTB();
    char inChar = input.value;

    if (inChar == 'a') {
        if (!portBvalue) {
            inChar = 'b';
        } else {
            inChar = 'c';
        }
    }

    switch (inChar) {
        case 'b':
            setPORTB(0xFF);
            sendLine("Toggled LED on");
            return TOGGLE_LED_MENU;
        case 'c':
            setPORTB(0x00);
            sendLine("Toggled LED off");
            return TOGGLE_LED_MENU;
        case 'q':
            return MAIN_MENU;
        default:
            sendUnknownCharMsg(input.value);
            return TOGGLE_LED_MENU;
    }
}

void printTemperature() {
    int8_t temp_msb;
    twireadsingle(DS3231_ADDR, 0x11, &temp_msb);
    uint8_t temp_lsb;
    twireadsingle(DS3231_ADDR, 0x12, &temp_lsb);
    temp_lsb >>= 6;

    char *data = "Temperature:       ";
    uint8_t index = sizeof("Temperature: ") - 1;
    if (temp_lsb < 0) {
        data[index++] = '-';
    }

    data[index++] = (temp_msb / 10) + ASCII_DECIMAL_OFFSET;
    data[index++] = (temp_msb % 10) + ASCII_DECIMAL_OFFSET;
    data[index++] = '.';
    if (temp_lsb == 0b00) {
        data[index++] = '0';
        data[index++] = '0';
    } else if (temp_lsb == 0b01) {
        data[index++] = '2';
        data[index++] = '5';
    } else if (temp_lsb == 0b10) {
        data[index++] = '5';
        data[index++] = '0';
    } else if (temp_lsb == 0b11) {
        data[index++] = '7';
        data[index++] = '5';
    }
    data[index] = 0;
    sendLine(data);
}