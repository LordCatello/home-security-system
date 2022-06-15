#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "rtc.h"

#define ADDRESS_SIZE				1
#define DATA_TRANSFER_SIZE			7

#define DS1307_ADDRESS 				0xD0
#define MAX_RETRY					3
#define MAX_INIT_DELAY_MS			10000

#define DS1307_SECONDS				0x00


typedef struct rtc_s {
	bool is_initialized;
	I2C_HandleTypeDef *i2c;
} rtc_t;


static rtc_t rtc = {false};


/* DECLARATION OF THE PRIVATE FUNCTIONS */
/*
 * Convert BCD to decimal
 */
static uint8_t bcd2dec(uint8_t val);


/*
 * Convert decimal to BCD
 */
static uint8_t dec2bcd(uint8_t val);


/* DEFINITION OF THE PRIVATE FUNCTIONS */

static uint8_t bcd2dec(uint8_t val)
{
    uint8_t res = ((val / 16 * 10) + (val % 16));
    return res;
}


static uint8_t dec2bcd(uint8_t val)
{
    uint8_t res = ((val / 10 * 16) + (val % 10));
    return res;
}


/* DEFINITION OF THE INTERFACE FUNCTIONS */

int rtc_init(I2C_HandleTypeDef *i2c)
{
	if (rtc.is_initialized) {
		return ERROR;
	}

	rtc.i2c = i2c;

	if(HAL_I2C_IsDeviceReady(rtc.i2c, DS1307_ADDRESS, MAX_RETRY, MAX_INIT_DELAY_MS) != HAL_OK)
	{
		return ERROR;
	}

	rtc.is_initialized = true;

	return SUCCESS;
}


int rtc_get_datetime(datetime_t *datetime, uint32_t max_wait_time_ms)
{
	if (!rtc.is_initialized) {
		return ERROR;
	}

	uint8_t data[DATA_TRANSFER_SIZE];

	if(HAL_I2C_Mem_Read(rtc.i2c, DS1307_ADDRESS, DS1307_SECONDS,
		       	   	    ADDRESS_SIZE, data, DATA_TRANSFER_SIZE, max_wait_time_ms) != HAL_OK)
	{
		return ERROR;
	}

	datetime->seconds = bcd2dec(data[0]);
	datetime->minutes = bcd2dec(data[1]);
	datetime->hours = bcd2dec(data[2]);
	datetime->day_month = bcd2dec(data[4]);
	datetime->month = bcd2dec(data[5]);
	datetime->year = bcd2dec(data[6]);

	return SUCCESS;
}


int rtc_set_datetime(const datetime_t *datetime, uint32_t max_wait_time_ms)
{
	if (!rtc.is_initialized) {
		return ERROR;
	}

	uint8_t data[DATA_TRANSFER_SIZE];

	data[0] = dec2bcd(datetime->seconds);
	data[1] = dec2bcd(datetime->minutes);

	data[2] = dec2bcd(datetime->hours);
	data[2] &= 63;	// 00111111 - selecting 24h mode
					// verify that works also in the 12h mode
	data[3] = 0;
	data[4] = dec2bcd(datetime->day_month);
	data[5] = dec2bcd(datetime->month);
	data[6] = dec2bcd(datetime->year);

	return HAL_I2C_Mem_Write(rtc.i2c, DS1307_ADDRESS,  DS1307_SECONDS,
							 ADDRESS_SIZE, data, DATA_TRANSFER_SIZE, max_wait_time_ms);
}


int rtc_get_datetime_string(char* datetime_string, size_t size, uint32_t max_wait_time_ms) {
	if (!rtc.is_initialized) {
		return ERROR;
	}

	if (size < DATETIME_STRING_SIZE) {
		return ERROR;
	}

	datetime_t datetime;
	int res;

	res = rtc_get_datetime(&datetime, max_wait_time_ms);

	if (res != SUCCESS) {
		return res;
	}

	rtc_from_datetime_to_string(&datetime, datetime_string, size);

	return SUCCESS;
}


int rtc_from_datetime_to_string(const datetime_t* datetime, char *datetime_string, size_t size) {
	unsigned int seconds = datetime->seconds;
	unsigned int minutes = datetime->minutes;
	unsigned int hours = datetime->hours;
	unsigned int day_month = datetime->day_month;
	unsigned int month = datetime->month;
	unsigned int year = datetime->year;

	snprintf(datetime_string, DATETIME_STRING_SIZE, "%02u/%02u/20%02u %02u:%02u:%02u", day_month, month, year, hours, minutes, seconds);

	return SUCCESS;
}


int rtc_destroy() {
	if (!rtc.is_initialized) {
		return ERROR;
	}

	rtc.is_initialized = false;

	return SUCCESS;
}
