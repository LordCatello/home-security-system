#include <stdbool.h>
#include "keypad.h"
#include "command_controller.h"
#include "logger.h"

#define ROWS (4)
#define COLUMNS (4)

// KEYS //
static const uint8_t keys[ROWS][COLUMNS] = {
		{ '1', '2', '3', 'A' },
		{ '4', '5', '6', 'B' },
		{ '7', '8', '9', 'C' },
		{ '*', '0', '#', 'D' }
};

// KEYPAD //
typedef struct keypad_s {
	bool is_initialized;
	TIM_HandleTypeDef *timer;
	bool counting;
	pin_t rows[4];
	pin_t columns[4];
	CONSUME_CHARACTER_FUNCTION consume_character;
} keypad_t;

static keypad_t keypad = {false};

// STATIC FUNCTIONS //

static int keypad_set_one_row_high(int high_row) {
	if (!keypad.is_initialized) {
		return ERROR;
	}

	for (int i = 0; i < ROWS; i++) {
		if (i != high_row)
			HAL_GPIO_WritePin(keypad.rows[i].port, keypad.rows[i].number, GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(keypad.rows[i].port, keypad.rows[i].number, GPIO_PIN_SET);
	}

	return SUCCESS;
}

static int keypad_set_all_rows_high(){
	if (!keypad.is_initialized) {
		return ERROR;
	}

	for (int i = 0; i < ROWS; i++) {
		HAL_GPIO_WritePin(keypad.rows[i].port, keypad.rows[i].number, GPIO_PIN_SET);
	}

	return SUCCESS;
}

// PUBLIC FUNCTIONS //

int keypad_init(TIM_HandleTypeDef *keypad_timer, uint64_t timer_clock_hz, pin_t rows[], size_t n_rows, pin_t columns[], size_t n_columns, CONSUME_CHARACTER_FUNCTION consume_character_callback) {
	if (keypad.is_initialized) {
		return ERROR;
	}

	if(n_rows != ROWS || n_columns != COLUMNS) {
		return ERROR;
	}

	keypad.counting = false;
	memcpy(keypad.rows, rows, ROWS * sizeof(pin_t));
	memcpy(keypad.columns, columns, COLUMNS * sizeof(pin_t));

	keypad.consume_character = consume_character_callback;
	keypad.timer = keypad_timer;

	if (set_timer_period(keypad.timer, 50, timer_clock_hz) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return ERROR;
	}

	keypad.is_initialized = true;

	if (keypad_set_all_rows_high() != SUCCESS) {
		logger_formatted_print("Unable to set all the rows high", UART_DELAY);
		return ERROR;
	}

	return SUCCESS;
}

void keypad_handle_timer_interrupt(TIM_HandleTypeDef *keypad_htim) {
	if (keypad_htim->Instance == keypad.timer->Instance) {
		for (int i = 0; i < COLUMNS; i++) {
			if (HAL_GPIO_ReadPin(keypad.columns[i].port, keypad.columns[i].number) == GPIO_PIN_SET) {
				for (int j = 0; j < ROWS; j++) {
					if (keypad_set_one_row_high(j) != SUCCESS) {
						logger_formatted_print("Unable to set the row high", UART_DELAY);
						return;
					}


					if (HAL_GPIO_ReadPin(keypad.columns[i].port, keypad.columns[i].number) == GPIO_PIN_SET){
						keypad.consume_character(keys[j][i]);
						goto exit_point_keypad;
					}
				}
			}
		}

		exit_point_keypad:
		// Procedure for accepting new inputs
		if (keypad_set_all_rows_high() != SUCCESS) {
			logger_formatted_print("Unable to set all the rows high", UART_DELAY);
			return;
		}
		// Before accepting new inputs, I need to discard any interrupts I could have triggered during the search of the row
		for(int i = 0; i < COLUMNS; i++) {
			__HAL_GPIO_EXTI_CLEAR_IT(keypad.columns[i].number);
		}

		keypad.counting = false;
		HAL_TIM_Base_Stop_IT(keypad_htim);
		return;
	}
}

void keypad_handle_gpio_interrupt(uint16_t GPIO_Pin) {
	if(GPIO_Pin == keypad.columns[0].number ||
	   GPIO_Pin == keypad.columns[1].number ||
	   GPIO_Pin == keypad.columns[2].number ||
	   GPIO_Pin == keypad.columns[3].number) {
		if (!keypad.counting) {
			keypad.counting = true;
			__HAL_TIM_CLEAR_FLAG(keypad.timer, TIM_FLAG_UPDATE); // Avoids the first call of the period elapsed callback on the Start_IT call
			HAL_TIM_Base_Start_IT(keypad.timer);
		}
	}
}


int keypad_destroy() {
	if (!keypad.is_initialized) {
		return ERROR;
	}

	HAL_TIM_Base_Stop_IT(keypad.timer);

	keypad.is_initialized = false;

	return SUCCESS;
}
