#ifndef INC_PHOTORESISTOR_LASER_H_
#define INC_PHOTORESISTOR_LASER_H_

/*
 * Barrier sensor module.
 *
 * It manages the barrier sensor, changing its state accordingly.
 */
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "common.h"

#define BARRIER_SENSOR_THRESHOLD (500)

int photoresistor_laser_init(pin_t laser_pin, pin_t photoresistor_pin, uint8_t init_state, int8_t delay, uint8_t alarm_duration, TIM_HandleTypeDef *sample_timerp,
							TIM_HandleTypeDef *alarm_timerp, ADC_HandleTypeDef *adc, uint64_t sample_timer_clock_hz, uint64_t alarm_timer_clock_hz);

int photoresistor_laser_destroy();

/*
 * Accordingly to the value of the photoresistor, starts and stops the timer in
 * order to change the state correctly.
 * Non-blocking.
 */
void photoresistor_laser_handle_adc_interrupt();

int photoresistor_laser_enable();

int photoresistor_laser_disable();

void photoresistor_laser_handle_timer_interrupt(TIM_HandleTypeDef *htimp);

uint8_t photoresistor_laser_get_actual_state();


#endif /* INC_PHOTORESISTOR_LASER_H_ */
