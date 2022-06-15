#include "common.h"
#include "logger.h"

#include <stdio.h>


int set_timer_period(TIM_HandleTypeDef *timer, uint32_t period_ms, uint64_t clock_hz) {
	uint32_t prescaler = clock_hz / 1000 - 1;

	if (prescaler > 0xFFFF) {
		return ERROR;
	}

	uint16_t period;
	if (period_ms > 0xFFFF) {
		char message[256];
		snprintf(message, 256, "The requested period (%ul ms) is too large. The maximum period (%u ms) will be used.", period_ms, 0xFFFF);
		logger_formatted_print(message, UART_DELAY);
		period = 0XFFFF;
	} else {
		period = period_ms;
	}

	if (period == 0) {
		period = 1;
	}

	__HAL_TIM_SET_PRESCALER(timer, prescaler);
	__HAL_TIM_SET_AUTORELOAD(timer, period - 1);

	return SUCCESS;
}
