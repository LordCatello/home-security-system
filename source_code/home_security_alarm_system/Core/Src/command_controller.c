#include "command_controller.h"
#include "common.h"
#include "buzzer.h"
#include <stdbool.h>
#include <stdio.h>
#include "PIR.h"
#include "photoresistor_laser.h"
#include "logger.h"
#include "led.h"

#define FIRST_CHAR 				('#')
#define ACTIVATION_CHAR 		('#')
#define DEACTIVATION_CHAR 		('*')
#define AREA_ALARM 				('A')
#define BARRIER_ALARM 			('B')
#define BOTH_ALARMS 			('C')
#define ENTIRE_SYSTEM 			('D')

#define LED_BLINK_INTERVAL_MS 	(500)

// PROTOTYPES //
static void command_evaluate();

// SOUNDS //
static const sound_t key_pressed_sound[] = {{80, 20}};
static const size_t key_pressed_sound_size = 1;

static const sound_t invalid_command_sound[] = {{100, 100}, {100, 100}, {100, 100}};
static const size_t invalid_command_sound_size = 3;

static const sound_t success_sound[] = {{1000, 50}};
static const size_t success_sound_size = 1;

static const sound_t wrong_pin_sound[] = {{350, 350}, {350, 350}, {350, 350}};
static const size_t wrong_pin_sound_size = 3;

// COMMAND STRUCTURE //
typedef struct command_s {
	uint8_t pin[PIN_SIZE];
	uint8_t command_scope;
	uint8_t command_type;
}command_t;

typedef struct command_controller_s {
	bool is_initialized;
	uint8_t n_characters_read;// = 0
	command_t command;
	uint8_t true_pin[PIN_SIZE];
} command_controller_t;

static command_controller_t command_controller = {false};

void command_controller_parse_character(char read_character){

	if(command_controller.n_characters_read == 0 && read_character != FIRST_CHAR){
		buzzer_play(invalid_command_sound, invalid_command_sound_size);
		logger_formatted_print("Invalid command.", UART_DELAY);
		return;
	}
	if(command_controller.n_characters_read > 0 && command_controller.n_characters_read < 5 && (read_character < '0' || read_character > '9')){
		buzzer_play(invalid_command_sound, invalid_command_sound_size);
		logger_formatted_print("Invalid command.", UART_DELAY);
		command_controller.n_characters_read = 0;
		return;
	}
	if(command_controller.n_characters_read == 5 && (read_character < 'A' || read_character > 'D')){
		buzzer_play(invalid_command_sound, invalid_command_sound_size);
		logger_formatted_print("Invalid command.", UART_DELAY);
		command_controller.n_characters_read = 0;
		return;
	}
	if(command_controller.n_characters_read == 6 && (read_character != '#' && read_character != '*')){
		buzzer_play(invalid_command_sound, invalid_command_sound_size);
		logger_formatted_print("Invalid command.", UART_DELAY);
		command_controller.n_characters_read = 0;
		return;
	}


	if(command_controller.n_characters_read >= 1 && command_controller.n_characters_read <= 4){
		// Saving the inserted pin
		command_controller.command.pin[command_controller.n_characters_read - 1] = read_character;
	}else if(command_controller.n_characters_read == 5){
		// Saving the scope of the command (Area,Barrier,Both,System)
		command_controller.command.command_scope = read_character;
	}else if(command_controller.n_characters_read == 6){
		// Saving the type of the command (Active, Inactive)
		command_controller.command.command_type = read_character;

		command_evaluate();
		command_controller.n_characters_read = 0;
		return;
	}
	command_controller.n_characters_read++;
	buzzer_play(key_pressed_sound, key_pressed_sound_size);
	return;
}

static void command_evaluate() {
	// Verify PIN (avoiding timing attacks even though it would be faster to brute force it :-) )
	bool valid_pin = true;
	for(int i = 0; i < PIN_SIZE; i++) {
		if(command_controller.command.pin[i] != command_controller.true_pin[i]) {
			valid_pin = false;
		}
	}

	if(valid_pin) {

		if(command_controller.command.command_type == DEACTIVATION_CHAR){
			switch (command_controller.command.command_scope) {
				case AREA_ALARM:
					PIR_disable();
					logger_formatted_print("Deactivating area alarm.", UART_DELAY);
					break;

				case BARRIER_ALARM:
					photoresistor_laser_disable();
					logger_formatted_print("Deactivating barrier alarm.", UART_DELAY);
					break;

				case BOTH_ALARMS:
					PIR_disable();
					photoresistor_laser_disable();
					logger_formatted_print("Deactivating both alarms.", UART_DELAY);
					break;

				case ENTIRE_SYSTEM:
					led_on();
					buzzer_stop();
					PIR_disable();
					photoresistor_laser_disable();
					logger_disable_periodic_log();
					logger_formatted_print("Deactivating entire system.", UART_DELAY);
					break;
			}
		}else{
			led_blink(LED_BLINK_INTERVAL_MS);
			logger_enable_periodic_log(LOG_INTERVAL_MS);
			switch (command_controller.command.command_scope) {
				case AREA_ALARM:
					PIR_enable();
					logger_formatted_print("Activating area alarm.", UART_DELAY);
					break;

				case BARRIER_ALARM:
					photoresistor_laser_enable();
					logger_formatted_print("Activating barrier alarm.", UART_DELAY);
					break;

				case BOTH_ALARMS:
					PIR_enable();
					photoresistor_laser_enable();
					logger_formatted_print("Activating both alarms.", UART_DELAY);
					break;

				case ENTIRE_SYSTEM:
					PIR_enable();
					photoresistor_laser_enable();
					logger_formatted_print("Activating entire system.", UART_DELAY);
					break;
			}
		}
		buzzer_play(success_sound, success_sound_size);
	} else {
		buzzer_play(wrong_pin_sound, wrong_pin_sound_size);
		logger_formatted_print("Wrong pin.", UART_DELAY);
	}

}

int command_controller_init(uint8_t pin[], size_t pin_length) {
	if(command_controller.is_initialized){
		return ERROR;
	}

	if(pin_length != PIN_SIZE){
		return ERROR;
	}

	memcpy(command_controller.true_pin, pin, PIN_SIZE * sizeof(uint8_t));
	command_controller.n_characters_read = 0;
	command_controller.is_initialized = true;

	return SUCCESS;

}

int command_controller_destroy() {
	if(!command_controller.is_initialized){
		return ERROR;
	}
	command_controller.is_initialized = false;

	return SUCCESS;
}

