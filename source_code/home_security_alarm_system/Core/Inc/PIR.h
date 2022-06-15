#ifndef INC_PIR_H_
#define INC_PIR_H_
/*
 * Area sensor module.
 *
 * It manages the barrier sensor, changing its state accordingly.
 */

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "common.h"

int PIR_init(pin_t pin, uint8_t init_state, int8_t delay, TIM_HandleTypeDef *htimp, uint8_t alarm_duration, uint64_t timer_clock);

int PIR_disable();

int PIR_enable();

uint8_t PIR_get_actual_state();

int PIR_destroy();

void PIR_handle_timer_interrupt(TIM_HandleTypeDef *htim);

/*
 * Starts and stops the timer accordingly to the rising and falling
 * edge of the input, in order to change the state of the sensor correctly.
 */
void PIR_handle_gpio_interrupt(uint16_t GPIO_PIN);

#endif /* INC_PIR_H_ */
