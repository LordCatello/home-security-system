#ifndef INC_COMMAND_CONTROLLER_H_
#define INC_COMMAND_CONTROLLER_H_

/*
 * Command controller module.
 *
 * This module handles the commands inserted by the user.
 *
 *
 */
#include "stm32f4xx_hal.h"
#include <string.h>
#define PIN_SIZE (4)

/*
 * Handles the character inserted through the keypad.
 */
void command_controller_parse_character(char read_character);


int command_controller_init(uint8_t pin[], size_t pin_length);

int command_controller_destroy();

#endif /* INC_COMMAND_CONTROLLER_H_ */
