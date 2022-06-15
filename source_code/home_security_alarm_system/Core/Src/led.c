/* INCLUDES */
#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "common.h"
#include "led.h"
#include "logger.h"

typedef enum {
	ON,
	OFF,
	BLINK
} led_state_t;


typedef struct led_s {
	bool is_initialized;
	led_state_t state;
	pin_t pin;
	TIM_HandleTypeDef* timer;
	uint64_t clock_hz;
} led_t;


// The state has been declared static because a callback of an interrupt needs it
static led_t led = {false};


/* DEFINITION OF THE INTERFACE FUNCTIONS */

int led_init(pin_t pin, TIM_HandleTypeDef* timer, uint64_t clock_hz) {
	if (led.is_initialized) {
		return ERROR;
	}

	led.pin = pin;
	led.timer = timer;
	led.clock_hz = clock_hz;
	led.is_initialized = true;

	led_off();

	return SUCCESS;
}


int led_on() {
	if (!led.is_initialized) {
		return ERROR;
	}

	if (led.state == BLINK) {
		HAL_TIM_Base_Stop_IT(led.timer);
	}

	HAL_GPIO_WritePin(led.pin.port, led.pin.number, GPIO_PIN_SET);

	led.state = ON;

	return SUCCESS;
}


int led_off() {
	if (!led.is_initialized) {
		return ERROR;
	}

	if (led.state == BLINK) {
		HAL_TIM_Base_Stop_IT(led.timer);
	}

	HAL_GPIO_WritePin(led.pin.port, led.pin.number, GPIO_PIN_RESET);

	led.state = OFF;

	return SUCCESS;
}


int led_blink(uint16_t period_ms) {
	if (!led.is_initialized) {
		return ERROR;
	}

	led_off();

	if (set_timer_period(led.timer, period_ms, led.clock_hz) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return ERROR;
	}

	__HAL_TIM_CLEAR_FLAG(led.timer, TIM_FLAG_UPDATE);
	__HAL_TIM_SET_COUNTER(led.timer, 0);

	led.state = BLINK;

	HAL_TIM_Base_Start_IT(led.timer);

	return SUCCESS;
}


int led_blink_callback(TIM_HandleTypeDef *timer) {
	if (!led.is_initialized) {
		return ERROR;
	}

	if (timer->Instance == led.timer->Instance) {
		HAL_GPIO_TogglePin(led.pin.port, led.pin.number);
	}

	return SUCCESS;
}


int led_destroy() {
	if (!led.is_initialized) {
		return ERROR;
	}

	led_off();

	led.is_initialized = false;

	return SUCCESS;
}
