#include <common.h>
#include "buzzer.h"
#include "stm32f4xx_hal_tim.h"
#include "main.h"
#include "stm32f4xx.h"
#include <string.h>
#include <stdbool.h>
#include "logger.h"

typedef struct buzzer_s {
	bool is_initialized;
	sound_t sounds_to_play[MAX_SOUNDS];
	size_t current_size;
	size_t i;
	bool currently_playing;
	bool buzzing;
	TIM_HandleTypeDef *timer;
	pin_t pin;
	uint64_t timer_clock_hz;
} buzzer_t;


static buzzer_t buzzer = {false};


int buzzer_init(TIM_HandleTypeDef *htim, pin_t pin, uint64_t timer_clock_hz) {
	if (buzzer.is_initialized) {
		return ERROR;
	}

	buzzer.timer = htim;
	buzzer.pin = pin;
	buzzer.is_initialized = true;
	buzzer.timer_clock_hz = timer_clock_hz;

	return SUCCESS;
}


int buzzer_destroy() {
	if (!buzzer.is_initialized) {
		return ERROR;
	}

	if(buzzer_stop() != SUCCESS) {
		return ERROR;
	}

	buzzer.is_initialized = false;
	return SUCCESS;
}


int buzzer_stop() {
	if (!buzzer.is_initialized) {
		return ERROR;
	}

	HAL_TIM_Base_Stop_IT(buzzer.timer);
	buzzer.buzzing = false;
	buzzer.currently_playing = false;
	HAL_GPIO_WritePin(buzzer.pin.port, buzzer.pin.number, GPIO_PIN_RESET);

	return SUCCESS;
}


int buzzer_play(const sound_t *sounds, const size_t size) {
	if (!buzzer.is_initialized) {
		return ERROR;
	}

	if(size > MAX_SOUNDS || size < 1) {
		return BUZZER_INVALID_SIZE;
	}

	if(buzzer.currently_playing) {
		return BUZZER_ALREADY_PLAYING;
	}

	buzzer.currently_playing = true;

	// configure sounds to play in the future
	memcpy(buzzer.sounds_to_play, sounds, size * sizeof(sound_t));
	buzzer.current_size = size;
	buzzer.i = 0;
	buzzer.buzzing = false;

	HAL_TIM_Base_Start_IT(buzzer.timer);

	return SUCCESS;
}


void buzzer_handler(TIM_HandleTypeDef *htim) {
	if(htim->Instance == buzzer.timer->Instance){
		if(buzzer.buzzing) {
			// Shut down the buzzer for off_msecs
			HAL_GPIO_WritePin(buzzer.pin.port, buzzer.pin.number, GPIO_PIN_RESET);
			buzzer.buzzing = false;

			// tell the timer to start buzzing after off_msecs
			if (set_timer_period(buzzer.timer, buzzer.sounds_to_play[buzzer.i].off_msecs, buzzer.timer_clock_hz) != SUCCESS) {
				logger_formatted_print("Unable to set the timer period", UART_DELAY);
				return;
			}

			(buzzer.i)++;
			if(buzzer.i >= buzzer.current_size) {
				HAL_TIM_Base_Stop_IT(htim);
				buzzer.currently_playing = false;
			}

		} else {
			// Play the buzzer for on_msecs
			HAL_GPIO_WritePin(buzzer.pin.port, buzzer.pin.number, GPIO_PIN_SET);
			buzzer.buzzing = true;

			// tell the buzzer to stop buzzing after on_msecs
			if (set_timer_period(buzzer.timer, buzzer.sounds_to_play[buzzer.i].on_msecs, buzzer.timer_clock_hz) != SUCCESS) {
				logger_formatted_print("Unable to set the timer period", UART_DELAY);
				return;
			}
		}
	}
}
