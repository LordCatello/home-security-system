#include "stm32f4xx_hal.h"
#include "configuration.h"
#include "command_controller.h"
#include "external_memory.h"
#include "rtc.h"
#include "logger.h"

#include <stdbool.h>
#include <stdio.h>

#define MAX_EEPROM_WAIT_TIME_MS (2000)
#define DUMP_MESSAGE_SIZE		(512)
#define ERROR_MESSAGE_SIZE		(200)
#define MAX_ALARM_DURATION_S    (60)
#define MAX_SENSOR_DELAY_S      (60)

static void accept_user_configuration(configuration_t *configuration, const uint8_t *buffer);

int load_configuration(UART_HandleTypeDef *uart, configuration_t *configuration, uint32_t max_uart_wait_time_ms) {
	// Try reading from external memory
	bool read_from_eeprom = true;
	if(external_memory_read((uint8_t *) configuration, sizeof(configuration_t), MAX_EEPROM_WAIT_TIME_MS) != SUCCESS) {
		logger_formatted_print("No configuration was found in the external memory.", 1000);
		read_from_eeprom = false;
	}

	// If I read a configuration from the eeprom, I am expecting a PIN from the new UART configuration
	size_t expected_size = sizeof(configuration_t);
	if(read_from_eeprom) {
		expected_size += PIN_SIZE;
	}

	char message[128];
	snprintf(message, 128, "Waiting up to %lu seconds for a user configuration by UART.", max_uart_wait_time_ms / 1000);
	logger_formatted_print(message, 1000);

	uint8_t buffer[expected_size];
	if(HAL_UART_Receive(uart, buffer, expected_size, max_uart_wait_time_ms) != HAL_OK) {
		if(read_from_eeprom) {
			logger_formatted_print("No user configuration was sent. Using the EEPROM conf.", 1000);
		} else {
			configuration_t default_configuration = {
					.alarm_duration_s = 5,
					.area_sensor_delay_s = 0,
					.barrier_sensor_delay_s = 0,
					.user_pin = {'0', '0', '0','0'},
			};
			if(rtc_get_datetime(&(default_configuration.datetime), 1000) != SUCCESS) {
				logger_formatted_print("Unable to get current date from RTC module. Aborting.", 1000);
				return ERROR;
			}

			memcpy(configuration, &default_configuration, sizeof(configuration_t));
			logger_formatted_print("No user configuration was sent. Loaded default configuration.", 1000);
		}
	} else {
		// Verify PIN (avoiding timing attacks even though it would be faster to brute force it :-) )
		if(read_from_eeprom) {
			uint8_t *uart_pin = buffer + sizeof(configuration_t);

			bool valid_pin = true;
			for(int i = 0; i < PIN_SIZE; i++) {
				if(uart_pin[i] != configuration->user_pin[i]) {
					valid_pin = false;
				}
			}

			if(valid_pin) {
				accept_user_configuration(configuration, buffer);
				if(external_memory_save((uint8_t *)configuration, sizeof(configuration_t), MAX_EEPROM_WAIT_TIME_MS) != SUCCESS) {
					logger_formatted_print("Unable to save the user configuration on the EEPROM.", 1000);
				} else {
					logger_formatted_print("Saved the user configuration on the EEPROM successfully.", 1000);
				}

			} else {
				logger_formatted_print("The user configuration was rejected due to bad PIN. Using the EEPROM conf.", 1000);
			}
		} else {
			accept_user_configuration(configuration, buffer);
		}



	}

	return SUCCESS;
}

int dump_configuration(configuration_t *configuration, uint32_t max_uart_wait_time_ms) {
	char message[DUMP_MESSAGE_SIZE];
	char datetime_string[DATETIME_STRING_SIZE];

	rtc_from_datetime_to_string(&(configuration->datetime), datetime_string, DATETIME_STRING_SIZE);

	snprintf(message,
			DUMP_MESSAGE_SIZE,
			"Configuration dump\r\n"
			"User pin: %c%c%c%c\r\n"
			"area_sensor_delay_s: %u\r\n"
			"barrier_sensor_delay_s: %u\r\n"
			"alarm_duration_s: %u\r\n"
			"datetime: %s\r\n",
			configuration->user_pin[0], configuration->user_pin[1], configuration->user_pin[2], configuration->user_pin[3],
			configuration->area_sensor_delay_s,
			configuration->barrier_sensor_delay_s,
			configuration->alarm_duration_s,
			datetime_string);

	if (logger_formatted_print(message, max_uart_wait_time_ms) != SUCCESS) {
		return ERROR;
	}

	return SUCCESS;

}

static void accept_user_configuration(configuration_t *configuration, const uint8_t *buffer) {
	memcpy(configuration, buffer, sizeof(configuration_t));
	//check that both the alarm duration and the sensors delay are less or equal than 60 seconds
	if (configuration->alarm_duration_s > MAX_ALARM_DURATION_S){
		configuration->alarm_duration_s = MAX_ALARM_DURATION_S;
		char error_message[ERROR_MESSAGE_SIZE];
		snprintf(error_message, ERROR_MESSAGE_SIZE, "The specified alarm duration is too large. The maximum one (%u) will be used", MAX_ALARM_DURATION_S);
		logger_formatted_print(error_message, 1000);
	}

	if (configuration->area_sensor_delay_s > MAX_SENSOR_DELAY_S){
		configuration->area_sensor_delay_s = MAX_SENSOR_DELAY_S;
		char error_message[ERROR_MESSAGE_SIZE];
		snprintf(error_message, ERROR_MESSAGE_SIZE, "The specified area sensor delay is too large. The maximum one (%u) will be used", MAX_SENSOR_DELAY_S);
		logger_formatted_print(error_message, 1000);
	}

	if (configuration->barrier_sensor_delay_s > MAX_SENSOR_DELAY_S){
		configuration->barrier_sensor_delay_s = MAX_SENSOR_DELAY_S;
		char error_message[ERROR_MESSAGE_SIZE];
		snprintf(error_message, ERROR_MESSAGE_SIZE, "The specified barrier sensor delay is too large. The maximum one (%u) will be used", MAX_SENSOR_DELAY_S);
		logger_formatted_print(error_message, 1000);
	}

	logger_formatted_print("The user configuration was accepted.", 1000);
	// save the new date
	if (rtc_set_datetime(&(configuration->datetime), 1000) != SUCCESS) {
		logger_formatted_print("Unable to save the date on the RTC.", 1000);
	} else {
		logger_formatted_print("New date successfully saved on the RTC.", 1000);
	}
}
