#ifndef INC_COMMON_H_
#define INC_COMMON_H_

/*
 * This module contains elements shared among other modules.
 *
 */

#include "stm32f4xx_hal.h"

#define INACTIVE 		(0)
#define ACTIVE 			(1)
#define ALARMED 		(2)
#define DELAYED 		(3)

#define UNINITIALIZED 	(4)

#define UART_DELAY 		(1000)

#define LOG_INTERVAL_MS (10000)

typedef struct pin_s {
	GPIO_TypeDef *port;
	uint16_t number;
} pin_t;

/**
 * Returns ERROR if the prescaler is too large (the timer register is too small).
 * The max value of the period is 0xFFFF.
 */
int set_timer_period(TIM_HandleTypeDef *timer, uint32_t period_ms, uint64_t clock_hz);

#endif /* INC_COMMON_H_ */
