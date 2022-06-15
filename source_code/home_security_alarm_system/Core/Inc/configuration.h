/*
 * configuration.h
 *
 *  Created on: Jun 7, 2020
 *      Author: LordCatello
 */

#ifndef INC_CONFIGURATION_H_
#define INC_CONFIGURATION_H_

/*
 *
 *  Configuration module
 *
 *  This module handles the configuration loading process during the boot,
 *  choosing between the default one,the external memory one and the one sent by the user through UART.
 *
 */
#include "stm32f4xx_hal.h"
#include "rtc.h"
#include "command_controller.h"

typedef struct configuration_s {
	uint8_t user_pin[PIN_SIZE];  // utf-8 encoding (for pin only)
	uint8_t area_sensor_delay_s;
	uint8_t barrier_sensor_delay_s;
	uint8_t alarm_duration_s;
	datetime_t datetime;

} configuration_t;


/*
 * Blocking.
 */
int load_configuration(UART_HandleTypeDef *uart, configuration_t *configuration, uint32_t max_uart_wait_time_ms);

int dump_configuration(configuration_t *configuration, uint32_t max_uart_wait_time_ms);


#endif /* INC_CONFIGURATION_H_ */
