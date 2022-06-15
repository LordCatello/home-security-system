#include "alarm_sound_controller.h"
#include <stdbool.h>
#include "common.h"
#include "buzzer.h"
#include "PIR.h"
#include "photoresistor_laser.h"


static const sound_t pir_alarm_sound[] = {{200, 100}, {500, 100}};
static const size_t pir_alarm_sound_size = 2;

static const sound_t photoresistor_laser_alarm_sound[] = {{150, 100}};
static const size_t photoresistor_laser_alarm_sound_size = 1;

static const sound_t both_alarm_sound[] = {{100, 100}, {200, 100}, {400, 100}};
static const size_t both_alarm_sound_size = 3;


typedef struct alarm_sound_controller_s {
	bool is_initialized;
	bool is_enabled;
	TIM_HandleTypeDef *htim;
} alarm_sound_controller_t;

static alarm_sound_controller_t controller = {false};

int alarm_sound_contoller_init(TIM_HandleTypeDef *htim, uint16_t period_ms, uint64_t timer_clock_hz) {
	if(controller.is_initialized) {
		return ERROR;
	}
	if (set_timer_period(htim, period_ms, timer_clock_hz) != SUCCESS){
		return ERROR;
	}
	controller.htim = htim;
	controller.is_initialized = true;
	return alarm_sound_controller_enable();
}

void alarm_sound_controller_handler(TIM_HandleTypeDef *htim) {
	if(htim->Instance == controller.htim->Instance && controller.is_initialized && controller.is_enabled) {
		if(PIR_get_actual_state() == ALARMED && photoresistor_laser_get_actual_state() != ALARMED) {
			buzzer_play(pir_alarm_sound, pir_alarm_sound_size);
		} else if(PIR_get_actual_state() != ALARMED && photoresistor_laser_get_actual_state() == ALARMED) {
			buzzer_play(photoresistor_laser_alarm_sound, photoresistor_laser_alarm_sound_size);
		} else if(PIR_get_actual_state() == ALARMED && photoresistor_laser_get_actual_state() == ALARMED) {
			buzzer_play(both_alarm_sound, both_alarm_sound_size);
		}
	}
}

int alarm_sound_controller_destroy(){
	if(!controller.is_initialized){
		return ERROR;
	}
	if (alarm_sound_controller_disable() != SUCCESS){
		return ERROR;
	}
	controller.is_initialized = false;
	return SUCCESS;
}

int alarm_sound_controller_enable() {
	if(!controller.is_initialized || controller.is_enabled) {
		return ERROR;
	}
	controller.is_enabled = true;
	HAL_TIM_Base_Start_IT(controller.htim);
	return SUCCESS;
}

int alarm_sound_controller_disable() {
	if(!controller.is_initialized || !controller.is_enabled) {
		return ERROR;
	}
	controller.is_enabled = false;
	HAL_TIM_Base_Stop_IT(controller.htim);
	return SUCCESS;
}
