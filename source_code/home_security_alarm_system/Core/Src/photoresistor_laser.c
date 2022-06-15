#include "photoresistor_laser.h"
#include "logger.h"

struct photoresistor_laser_s{
	bool is_initialized;
	pin_t laser_pin;
	pin_t photoresistor_pin;
	uint8_t state;
	uint8_t alarm_delay_time;
	uint8_t alarm_duration_time;
	TIM_HandleTypeDef *sample_timerp;
	TIM_HandleTypeDef *alarm_timerp;
	ADC_HandleTypeDef *adc;
	uint64_t sample_timer_clock_hz;
	uint64_t alarm_timer_clock_hz;
};

typedef struct photoresistor_laser_s photoresistor_laser_t;

static photoresistor_laser_t sensor = {false};

// Declaration of static functions
static void start_timer(TIM_HandleTypeDef *htimp);
static void stop_timer(TIM_HandleTypeDef *htimp);
static void photoresistor_laser_start_alarm_condition();
static void photoresistor_laser_stop_alarm_condition();

int photoresistor_laser_init(pin_t laser_pin, pin_t photoresistor_pin, uint8_t init_state, int8_t delay, uint8_t alarm_duration, TIM_HandleTypeDef *sample_timerp, TIM_HandleTypeDef *alarm_timerp, ADC_HandleTypeDef *adc, uint64_t sample_timer_clock_hz, uint64_t alarm_timer_clock_hz){
	if(sensor.is_initialized) {
		return ERROR;
	}

	sensor.laser_pin = laser_pin;
	sensor.photoresistor_pin = photoresistor_pin;
	sensor.state = init_state;
	sensor.alarm_delay_time = delay + 1; // One second for alarming the sensor and the remeaning is the delay
	sensor.alarm_duration_time = alarm_duration;
	sensor.sample_timerp = sample_timerp;
	sensor.alarm_timerp = alarm_timerp;
	sensor.adc = adc;
	sensor.is_initialized = true;
	sensor.sample_timer_clock_hz = sample_timer_clock_hz;
	sensor.alarm_timer_clock_hz = alarm_timer_clock_hz;

	if (set_timer_period(sensor.sample_timerp, 50, sensor.sample_timer_clock_hz) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return ERROR;
	}

	if(sensor.state == ACTIVE){
		HAL_GPIO_WritePin(sensor.laser_pin.port, sensor.laser_pin.number, GPIO_PIN_SET);
		start_timer(sensor.sample_timerp);
	}

	return SUCCESS;
}



void photoresistor_laser_handle_adc_interrupt(){
	if(!sensor.is_initialized) {
		return;
	}
	uint32_t light = HAL_ADC_GetValue(sensor.adc);
	if(light <= BARRIER_SENSOR_THRESHOLD && sensor.state == DELAYED){
		photoresistor_laser_stop_alarm_condition();
	} else if (light > BARRIER_SENSOR_THRESHOLD && sensor.state == ACTIVE){
		photoresistor_laser_start_alarm_condition();
	}
}

void photoresistor_laser_handle_timer_interrupt(TIM_HandleTypeDef *htimp){
	if(sensor.is_initialized){
		if(htimp->Instance == sensor.sample_timerp->Instance){
			HAL_ADC_Start_IT(sensor.adc);
		}
		if(htimp->Instance == sensor.alarm_timerp->Instance){
			if(sensor.state == DELAYED){
				sensor.state = ALARMED;
				stop_timer(sensor.sample_timerp); // stop the timer in order to avoid reading from the adc during the alarm_duration

				if (set_timer_period(sensor.alarm_timerp, sensor.alarm_duration_time*1000, sensor.alarm_timer_clock_hz) != SUCCESS) {
					logger_formatted_print("Unable to set the timer period", UART_DELAY);
					return;
				}

				start_timer(sensor.alarm_timerp);

				logger_formatted_print("Alarm detected on barrier sensor.", UART_DELAY);

			} else if (sensor.state == ALARMED){
				sensor.state = ACTIVE;
				stop_timer(sensor.alarm_timerp);
				start_timer(sensor.sample_timerp);
			}
		}
	}
}


static void photoresistor_laser_start_alarm_condition(){
	sensor.state = DELAYED;

	if (set_timer_period(sensor.alarm_timerp, sensor.alarm_delay_time*1000, sensor.alarm_timer_clock_hz) != SUCCESS) {
		logger_formatted_print("Unable to set the timer period", UART_DELAY);
		return;
	}

	start_timer(sensor.alarm_timerp);
}

static void photoresistor_laser_stop_alarm_condition(){
	sensor.state = ACTIVE;
	stop_timer(sensor.alarm_timerp);
}

int photoresistor_laser_disable(){
	if(!sensor.is_initialized) {
		return ERROR;
	}

	HAL_GPIO_WritePin(sensor.laser_pin.port, sensor.laser_pin.number, GPIO_PIN_RESET);
	sensor.state=INACTIVE;
	stop_timer(sensor.sample_timerp);
	stop_timer(sensor.alarm_timerp);

	return SUCCESS;
}

int photoresistor_laser_enable(){
	if(!sensor.is_initialized) {
		return ERROR;
	}
	sensor.state=ACTIVE;
	HAL_GPIO_WritePin(sensor.laser_pin.port, sensor.laser_pin.number, GPIO_PIN_SET);
	start_timer(sensor.sample_timerp);

	return SUCCESS;
}

uint8_t photoresistor_laser_get_actual_state(){
	if(sensor.is_initialized){
		return sensor.state;
	}
	return UNINITIALIZED;
}



static void start_timer(TIM_HandleTypeDef *htimp){
	__HAL_TIM_CLEAR_FLAG(htimp, TIM_FLAG_UPDATE);
	__HAL_TIM_SET_COUNTER(htimp, 0);
	HAL_TIM_Base_Start_IT(htimp);
}

static void stop_timer(TIM_HandleTypeDef *htimp){
	HAL_TIM_Base_Stop_IT(htimp);
}
