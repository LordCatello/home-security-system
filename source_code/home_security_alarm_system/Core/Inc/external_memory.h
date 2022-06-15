#ifndef INC_EXTERNAL_MEMORY_H_
#define INC_EXTERNAL_MEMORY_H_

/*
 * External memory module.
 *
 * This module allows to transmit and receive data to/from external memory.
 *
 */

#include "stm32f4xx_hal.h"

int external_memory_init(I2C_HandleTypeDef *i2c);

int external_memory_destroy();

/*
 * Blocking
 */
int external_memory_save(uint8_t *buffer, size_t size, uint32_t max_wait_time_ms);

/*
 * Blocking
 */
int external_memory_read(uint8_t *buffer, size_t size, uint32_t max_wait_time_ms);

#endif /* INC_EXTERNAL_MEMORY_H_ */
