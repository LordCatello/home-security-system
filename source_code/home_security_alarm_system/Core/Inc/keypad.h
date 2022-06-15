#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

/*
 * Keypad module.
 *
 * This module allows to understand which key was pressed on the keypad.
 *
 */

#include "common.h"
#include "stm32f4xx_hal.h"

#include <string.h>

/*
 * The type of the callback.
 */
typedef void (*CONSUME_CHARACTER_FUNCTION)(char);

/*
 * Initializes the keypad.
 * It requires a callback that will be called every time a key is pressed.
 */
int keypad_init(TIM_HandleTypeDef *keypad_timer, uint64_t timer_clock_hz, pin_t rows[], size_t n_rows, pin_t columns[], size_t n_columns, CONSUME_CHARACTER_FUNCTION consume_character_callback);

void keypad_handle_timer_interrupt(TIM_HandleTypeDef *keypad_htim);

/*
 * Called when a key is pressed.
 * In order to avoid the bouncing of the input signal without active waiting,
 * a timer is used.
 */
void keypad_handle_gpio_interrupt(uint16_t GPIO_Pin);

int keypad_destroy();

#endif /* INC_KEYPAD_H_ */
