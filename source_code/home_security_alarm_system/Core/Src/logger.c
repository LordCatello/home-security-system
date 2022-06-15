#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "common.h"
#include "logger.h"
#include "rtc.h"
#include "PIR.h"
#include "photoresistor_laser.h"


typedef struct logger_s {
	bool is_initialized;
	bool is_periodic_log_enabled;
	UART_HandleTypeDef *uart;
	TIM_HandleTypeDef* timer;
	uint64_t clock_hz;
} logger_t;


static logger_t logger = {false};

static const char LOG_STATE_STRINGS[][14] = {"Inactive", "Active", "Alarmed", "Delayed", "Uninitialized"};


/* DEFINITION OF THE INTERFACE FUNCTIONS */

int logger_init(UART_HandleTypeDef *uart, TIM_HandleTypeDef* timer, uint64_t clock_hz) {
	if (logger.is_initialized) {
		return ERROR;
	}

	logger.uart = uart;
	logger.timer = timer;
	logger.clock_hz = clock_hz;
	logger.is_periodic_log_enabled = false;

	logger.is_initialized = true;

	return SUCCESS;
}


int logger_print(char *message, uint32_t max_wait_time_ms) {
	if (!logger.is_initialized) {
		return ERROR;
	}

	return HAL_UART_Transmit(logger.uart, (uint8_t *) message, strlen(message), max_wait_time_ms);
}


int logger_formatted_print(char *message, uint32_t max_wait_time_ms) {
	if (!logger.is_initialized) {
		return ERROR;
	}

	int res = ERROR;

	char datetime_string[DATETIME_STRING_SIZE] = {'\0'};
	char formatted_datetime_string[DATETIME_STRING_SIZE + 10] = {'\0'};

	res = rtc_get_datetime_string(datetime_string, DATETIME_STRING_SIZE, 1000);
	if (res != SUCCESS) {
		return res;
	}

	sprintf(formatted_datetime_string, "%s - ", datetime_string);

	res = logger_print(formatted_datetime_string, max_wait_time_ms);

	if (res != HAL_OK) {
		return res;
	}

	res = logger_print(message, max_wait_time_ms);

	if (res != HAL_OK) {
		return res;
	}

	char suffix[3] = "\r\n";
	res = logger_print(suffix, max_wait_time_ms);

	return res;
}


int logger_enable_periodic_log(uint16_t period_ms) {
	if (!logger.is_initialized) {
		return ERROR;
	}

	if (logger.is_periodic_log_enabled) {
		return ERROR;
	}

	if (set_timer_period(logger.timer, period_ms, logger.clock_hz) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return ERROR;
	}

	__HAL_TIM_CLEAR_FLAG(logger.timer, TIM_FLAG_UPDATE);
	__HAL_TIM_SET_COUNTER(logger.timer, 0);

	HAL_TIM_Base_Start_IT(logger.timer);

	logger.is_periodic_log_enabled = true;

	return SUCCESS;
}


int logger_disable_periodic_log() {
	if (!logger.is_initialized) {
		return ERROR;
	}

	if (!logger.is_periodic_log_enabled) {
		return ERROR;
	}

	HAL_TIM_Base_Stop_IT(logger.timer);

	logger.is_periodic_log_enabled = false;

	return SUCCESS;
}


int logger_periodic_log_callback(TIM_HandleTypeDef *timer) {
	if (!logger.is_initialized) {
		return ERROR;
	}

	if (timer->Instance == logger.timer->Instance) {
		uint8_t PIR_state = PIR_get_actual_state();
		uint8_t photoresistor_laser_state = photoresistor_laser_get_actual_state();

		char message[256];
		snprintf(message, 256, "Area %s - Barrier %s", LOG_STATE_STRINGS[PIR_state], LOG_STATE_STRINGS[photoresistor_laser_state]);
		logger_formatted_print(message, 1000);
	}

	return SUCCESS;
}


int logger_destroy() {
	if (!logger.is_initialized) {
		return ERROR;
	}

	logger_disable_periodic_log();

	logger.is_initialized = false;

	return SUCCESS;
}
