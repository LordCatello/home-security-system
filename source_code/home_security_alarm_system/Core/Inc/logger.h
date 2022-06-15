#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

/*
 * Logger module.
 *
 * It allows to send messages through the UART interface.
 *
 */

/* INCLUDES */
#include <stdint.h>

#include "stm32f4xx_hal.h"

/* DECLARATION OF THE INTERFACE FUNCTIONS */

int logger_init(UART_HandleTypeDef *uart, TIM_HandleTypeDef* timer, uint64_t clock_hz);


/**
 * Sends a message through the UART interface.
 * Blocking
 */
int logger_print(char *message, uint32_t max_wait_time_ms);


/**
 * Sends a message through the UART interface adding as prefix the date-time and
 * as suffix \r\n.
 * Blocking
 */
int logger_formatted_print(char *message, uint32_t max_wait_time_ms);


int logger_enable_periodic_log(uint16_t period_ms);


int logger_disable_periodic_log();


/**
 *
 * Blocking
 */
int logger_periodic_log_callback(TIM_HandleTypeDef *timer);


int logger_destroy();

#endif /* INC_LOGGER_H_ */
