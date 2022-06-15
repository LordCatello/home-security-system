#ifndef INC_ALARM_SOUND_CONTROLLER_H_
#define INC_ALARM_SOUND_CONTROLLER_H_

/*
 * Alarm sound controller module.
 *
 * This module takes care of the alarms sound generation.
 * It calls the buzzer module in order to generate different sounds
 * according to the state of the sensors.
 *
 */

#include "stm32f4xx_hal.h"

/*
 * Initializes the state of the module with the timer used to query the state of the sensors.
 * It also calls the alarm_sound_controller_enable function.
 *
 */
int alarm_sound_contoller_init(TIM_HandleTypeDef *htim, uint16_t period_ms, uint64_t timer_clock_hz);


void alarm_sound_controller_handler(TIM_HandleTypeDef *htim);

int alarm_sound_controller_destroy();

/*
 * Enables the sound generation.
 * Non-blocking.
 */
int alarm_sound_controller_enable();

int alarm_sound_controller_disable();

#endif /* INC_ALARM_SOUND_CONTROLLER_H_ */
