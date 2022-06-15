#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_


/*
 * Buzzer module.
 *
 * This module is used to generate different sounds.
 *
 */


#include <common.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

#define MAX_SOUNDS 				(10)
#define BUZZER_INVALID_SIZE 	(-1)
#define BUZZER_ALREADY_PLAYING 	(-2)


/*
 * It represents a note played by the buzzer.
 *
 */
typedef struct sound_s {
	int on_msecs;
	int off_msecs;
} sound_t;


int buzzer_init(TIM_HandleTypeDef *htim, pin_t pin, uint64_t clock_hz);

int buzzer_destroy();

void buzzer_handler(TIM_HandleTypeDef *htim);

/*
 * Plays a sequence of sound_t, activating and deactivating the buzzer timer accordingly.
 * Ignores new calls if the buzzer is already playing a sequence, returning BUZZER_ALREADY_PLAYING.
 * Non-blocking.
 */
int buzzer_play(const sound_t *sounds, const size_t size);

int buzzer_stop();

#endif /* INC_BUZZER_H_ */
