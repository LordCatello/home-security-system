#ifndef INC_RTC_H_
#define INC_RTC_H_

/*
 * RTC module.
 *
 * It allows to read and write the datetime from/to a RTC.
 */
#include <stdint.h>

#include "stm32f4xx_hal.h"

#define DATETIME_STRING_SIZE 20

typedef struct datetime_s {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;

	uint8_t day_month;
	uint8_t month;
	uint8_t year; // in [0, 99]
} datetime_t;

/* DECLARATION OF THE INTERFACE FUNCTIONS */

int rtc_init(I2C_HandleTypeDef *i2c);


/**
 * Blocking
 */
int rtc_set_datetime(const datetime_t *datetime, uint32_t max_wait_time_ms);


/**
 * Blocking
 */
int rtc_get_datetime(datetime_t *datetime, uint32_t max_wait_time_ms);


/**
 * Blocking
 */
int rtc_get_datetime_string(char* datetime_string, size_t size, uint32_t max_wait_time_ms);


int rtc_from_datetime_to_string(const datetime_t* datetime, char *datetime_string, size_t size);


int rtc_destroy();

#endif /* INC_RTC_H_ */
