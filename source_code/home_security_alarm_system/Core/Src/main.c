/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rtc.h"
#include "logger.h"
#include "common.h"
#include "led.h"
#include "buzzer.h"
#include "keypad.h"
#include "external_memory.h"
#include "configuration.h"
#include "PIR.h"
#include "command_controller.h"
#include "photoresistor_laser.h"
#include "alarm_sound_controller.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLOCK_HZ 						(16000000) // 16MHz
#define MAX_UART_DELAY_MS				(1000)
#define MAX_CONFIGURATION_WAIT_TIME_MS  (30000)

#define KEYPAD_ROWS 					(4)
#define KEYPAD_COLUMNS 					(4)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_USART2_UART_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */

	// RTC //
	if (rtc_init(&hi2c1) != SUCCESS) {
		return ERROR;
	}

	// LOGGER //
	if (logger_init(&huart2, &htim1, CLOCK_HZ) != SUCCESS) {
		return ERROR;
	}
	logger_formatted_print("Logger and RTC modules successfully initialized.", MAX_UART_DELAY_MS);

	// LED //
	pin_t led_pin = {
			.port = GPIOA,
			.number = GPIO_PIN_5
	};
	if (led_init(led_pin, &htim2, CLOCK_HZ) != SUCCESS) {
		logger_formatted_print("Error while initializing the led module.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Led module successfully initialized.", MAX_UART_DELAY_MS);

	// BUZZER //
	pin_t buzzer_pin = {
			.port = GPIOA,
			.number = GPIO_PIN_8
	};
	if (buzzer_init(&htim3, buzzer_pin, CLOCK_HZ) != SUCCESS) {
		logger_formatted_print("Error while initializing the buzzer module.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Buzzer module successfully initialized.", MAX_UART_DELAY_MS);

	// EXTERNAL MEMORY //
	if (external_memory_init(&hi2c1) != SUCCESS) {
		logger_formatted_print("External memory not found.", MAX_UART_DELAY_MS);
	} else {
		logger_formatted_print("External memory module successfully initialized.", MAX_UART_DELAY_MS);
	}

	// CONFIGURATION //
	configuration_t configuration;
	if(load_configuration(&huart2, &configuration, MAX_CONFIGURATION_WAIT_TIME_MS) != SUCCESS) {
		return ERROR;
	}
	if (dump_configuration(&configuration, MAX_UART_DELAY_MS) != SUCCESS) {
		logger_formatted_print("Unable to dump the configuration", MAX_UART_DELAY_MS);
	}

	// COMMAND CONTROLLER //
	if(command_controller_init(configuration.user_pin, PIN_SIZE) != SUCCESS) {
		logger_formatted_print("Error while initializing the command controller module.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Command controller module successfully initialized.", MAX_UART_DELAY_MS);

	// KEYPAD //
	pin_t keypad_rows[] = {
			{
					.port = GPIOC,
					.number = GPIO_PIN_6
			},
			{
					.port = GPIOC,
					.number = GPIO_PIN_7
			},
			{
					.port = GPIOC,
					.number = GPIO_PIN_8
			},
			{
					.port = GPIOC,
					.number = GPIO_PIN_9
			},
	};
	pin_t keypad_columns[] = {
			{
					.port = GPIOA,
					.number = GPIO_PIN_10
			},
			{
					.port = GPIOA,
					.number = GPIO_PIN_11
			},
			{
					.port = GPIOA,
					.number = GPIO_PIN_12
			},
			{
					.port = GPIOA,
					.number = GPIO_PIN_13
			},
	};
	if(keypad_init(&htim4, CLOCK_HZ, keypad_rows, KEYPAD_ROWS, keypad_columns, KEYPAD_COLUMNS, command_controller_parse_character) != SUCCESS) {
		logger_formatted_print("Error while initializing the keypad module.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Keypad module successfully initialized.", MAX_UART_DELAY_MS);

	// PIR //
	pin_t PIR_pin = {
			.port = GPIOB,
			.number = GPIO_PIN_8
	};
	if(PIR_init(PIR_pin, INACTIVE, configuration.area_sensor_delay_s, &htim5, configuration.alarm_duration_s, CLOCK_HZ) != SUCCESS) {
		logger_formatted_print("Error while initializing the PIR area sensor.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("PIR area sensor successfully initialized.", MAX_UART_DELAY_MS);

	// PHOTORESISTOR //
	pin_t photoresistor_pin = {
			.port = GPIOA,
			.number = GPIO_PIN_0
	};
	pin_t laser_pin = {
			.port = GPIOA,
			.number = GPIO_PIN_9
	};
	if(photoresistor_laser_init(laser_pin, photoresistor_pin, INACTIVE, configuration.barrier_sensor_delay_s, configuration.alarm_duration_s, &htim10, &htim11, &hadc1, CLOCK_HZ, CLOCK_HZ) != SUCCESS) {
		logger_formatted_print("Error while initializing the photoresistor-laser barrier sensor.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Photoresistor-laser barrier sensor successfully initialized.", MAX_UART_DELAY_MS);

	// ALARM SOUND CONTROLLER //
	if(alarm_sound_contoller_init(&htim9, 100, CLOCK_HZ) != SUCCESS) {
		logger_formatted_print("Error while initializing the alarm sound controller.", MAX_UART_DELAY_MS);
		return ERROR;
	}
	logger_formatted_print("Alarm sound controller successfully initialized.", MAX_UART_DELAY_MS);


	// TURN ON THE LED //
	if (led_on() != SUCCESS) {
		logger_formatted_print("Unable to turn on the led", MAX_UART_DELAY_MS);
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
