cmake_minimum_required(VERSION 3.10)
project(arduinoUno C)

SET(MCU "atmega328p")
SET(SERIALDEV "/dev/ttyACM0")
SET(F_CPU "16000000")
# SET(CMAKE_VERBOSE_MAKEFILE 1)
# For some reason, these paths have to be absolute, otherwise
# CLion won't be able to find headers etc.
SET(CMAKE_C_COMPILER avr-gcc)
SET(CMAKE_CXX_COMPILER avr-g++)

SET(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU}")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

MACRO(AVR_CREATE_HEX name filename)
    ADD_CUSTOM_COMMAND(TARGET ${name} POST_BUILD COMMAND avr-objcopy ARGS -O binary -R.eeprom ${filename} ${name}.bin)
    ADD_CUSTOM_COMMAND(TARGET ${name} POST_BUILD COMMAND avrdude ARGS -p ${MCU} -c arduino -U flash:w:${name}.bin -P ${SERIALDEV})
ENDMACRO(AVR_CREATE_HEX)

add_executable(main src/main.c src/io/iocontrol.c src/io/iocontrol.h src/menu.c src/menu.h)
target_link_options(main PRIVATE "LINKER:-print-memory-usage")
AVR_CREATE_HEX(main main)

