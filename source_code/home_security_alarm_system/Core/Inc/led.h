#ifndef INC_LED_H_
#define INC_LED_H_

/*
 * Led module
 *
 * This module manages the behavior of the led.
 *
 */

/* INCLUDES */
#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "common.h"


/* DECLARATION OF THE INTERFACE FUNCTIONS */

int led_init(pin_t pin, TIM_HandleTypeDef* timer, uint64_t clock_hz);


int led_on();


int led_off();

/*
 * Non-blocking.
 */
int led_blink(uint16_t period_ms);


int led_blink_callback(TIM_HandleTypeDef *timer);

/*
 * Turns the led off and stops the blinking if enabled.
 */
int led_destroy();


#endif /* INC_LED_H_ */
