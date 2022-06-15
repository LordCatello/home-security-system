#include "external_memory.h"
#include <stdbool.h>

#define MAX_RETRY 			3
#define MAX_INIT_DELAY_MS 	1000

#define EEPROM_ADDRESS 		0xA0
#define DATA_ADDRESS 		0
#define ADDRESS_LENGTH 		2

typedef struct external_memory_s {
	bool is_initialized;
	I2C_HandleTypeDef *i2c;
} external_memory_t;

static external_memory_t external_memory = {false};

int external_memory_init(I2C_HandleTypeDef *i2c) {
	if (external_memory.is_initialized) {
		return ERROR;
	}

	external_memory.i2c = i2c;

	if(HAL_I2C_IsDeviceReady(external_memory.i2c, EEPROM_ADDRESS, MAX_RETRY, MAX_INIT_DELAY_MS) != HAL_OK) {
		return ERROR;
	}

	external_memory.is_initialized = true;

	return SUCCESS;
}


int external_memory_destroy() {
	if (!external_memory.is_initialized) {
		return ERROR;
	}

	external_memory.is_initialized = false;

	return SUCCESS;
}


int external_memory_save(uint8_t *buffer, size_t size, uint32_t max_wait_time_ms) {
	if (!external_memory.is_initialized) {
		return ERROR;
	}

	if (HAL_I2C_Mem_Write(external_memory.i2c, EEPROM_ADDRESS, DATA_ADDRESS, ADDRESS_LENGTH, buffer, size, max_wait_time_ms) != HAL_OK) {
		return ERROR;
	}

	return SUCCESS;
}


int external_memory_read(uint8_t *buffer, size_t size, uint32_t max_wait_time_ms) {
	if (!external_memory.is_initialized) {
		return ERROR;
	}

	if (HAL_I2C_Mem_Read(external_memory.i2c, EEPROM_ADDRESS, DATA_ADDRESS, ADDRESS_LENGTH, buffer, size, max_wait_time_ms) != HAL_OK) {
		return ERROR;
	}

	return SUCCESS;
}
