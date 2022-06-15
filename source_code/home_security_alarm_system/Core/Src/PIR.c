#include "PIR.h"
#include <stdint.h>
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

struct PIR_sensor_s
{
	bool is_initialized;
	pin_t pin;
	uint64_t timer_clock;
	GPIO_PinState last_value;
	uint8_t state;
	uint8_t delay_time;
	uint8_t alarm_duration;
	TIM_HandleTypeDef *htimp;

};

typedef struct PIR_sensor_s PIR_sensor_t;


static PIR_sensor_t sensor = {false};

static int PIR_start_alarm_condition();

static int PIR_stop_alarm_condition();

int PIR_init(pin_t pin, uint8_t init_state, int8_t delay, TIM_HandleTypeDef *htimp, uint8_t alarm_duration, uint64_t timer_clock) {
	if(sensor.is_initialized) {
		return ERROR;
	}

	sensor.is_initialized = true;
	sensor.pin = pin;
	sensor.last_value = 0;
	sensor.timer_clock = timer_clock;
	sensor.state = init_state;
	sensor.delay_time = delay + 1;
	sensor.htimp = htimp;
	sensor.alarm_duration = alarm_duration;

	return SUCCESS;

}

static int PIR_start_alarm_condition() {
	if(!sensor.is_initialized) {
		return ERROR;
	}

	if (set_timer_period(sensor.htimp, 1000 * sensor.delay_time, sensor.timer_clock) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return ERROR;
	}

	__HAL_TIM_CLEAR_FLAG(sensor.htimp, TIM_FLAG_UPDATE);
	__HAL_TIM_SET_COUNTER(sensor.htimp, 0);
	sensor.state = DELAYED;

	HAL_TIM_Base_Start_IT(sensor.htimp);

	return SUCCESS;
}

static int PIR_stop_alarm_condition() {
	if(!sensor.is_initialized) {
		return ERROR;
	}

	HAL_TIM_Base_Stop_IT(sensor.htimp);

	sensor.state = ACTIVE;

	return SUCCESS;
}

int PIR_disable() {
	if(!sensor.is_initialized) {
		return ERROR;
	}

	sensor.state = INACTIVE;
	HAL_TIM_Base_Stop_IT(sensor.htimp);

	return SUCCESS;
}

int PIR_destroy() {
	if(!sensor.is_initialized) {
		return ERROR;
	}

	if(PIR_disable() != SUCCESS) {
		return ERROR;
	}

	sensor.is_initialized = false;
	return SUCCESS;
}

int PIR_enable() {
	if(!sensor.is_initialized) {
		return ERROR;
	}
	sensor.state = ACTIVE;

	return SUCCESS;

}

uint8_t PIR_get_actual_state() {
	if (sensor.is_initialized) {
		return sensor.state;
	}

	return UNINITIALIZED;
}

void PIR_handle_gpio_interrupt(uint16_t GPIO_PIN) {
	if (GPIO_PIN == sensor.pin.number && sensor.is_initialized
			&& (sensor.state == ACTIVE || sensor.state == DELAYED)) {
		if (sensor.last_value == GPIO_PIN_RESET) {
			// on rising edge callback
			sensor.last_value = HAL_GPIO_ReadPin(sensor.pin.port,
					sensor.pin.number);
			PIR_start_alarm_condition();

		} else if (sensor.last_value == GPIO_PIN_SET) {
			// on falling edge callback
			sensor.last_value = HAL_GPIO_ReadPin(sensor.pin.port,
					sensor.pin.number);

			PIR_stop_alarm_condition();
		}
	}

}

void PIR_handle_timer_interrupt(TIM_HandleTypeDef *htim) {
	if (sensor.is_initialized && htim->Instance == sensor.htimp->Instance) {
		if (sensor.state == DELAYED) {
			sensor.state = ALARMED;

			if (set_timer_period(sensor.htimp, 1000 * sensor.alarm_duration, sensor.timer_clock) != SUCCESS) {
				logger_formatted_print("Unable to set the timer period", UART_DELAY);
				return;
			}

			__HAL_TIM_SET_COUNTER(sensor.htimp, 0);

			logger_formatted_print("Alarm detected on area sensor.", UART_DELAY);

		} else if (sensor.state == ALARMED) {


			HAL_TIM_Base_Stop_IT(sensor.htimp);
			// the pending bit is cleared in order to restore the original
			// configuration of the sensor. (Also to avoid the falling edge callback
			// to be called  uselessly)
			__HAL_GPIO_EXTI_CLEAR_IT(sensor.pin.number);
			sensor.last_value = HAL_GPIO_ReadPin(sensor.pin.port, sensor.pin.number);
			PIR_enable();

		}

	}
}

