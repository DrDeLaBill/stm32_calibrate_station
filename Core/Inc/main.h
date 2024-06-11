/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

int _write(int file, uint8_t *ptr, int len);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VALVE1_Pin GPIO_PIN_2
#define VALVE1_GPIO_Port GPIOC
#define VALVE2_Pin GPIO_PIN_3
#define VALVE2_GPIO_Port GPIOC
#define PUMP_Pin GPIO_PIN_0
#define PUMP_GPIO_Port GPIOA
#define LEVEL_Pin GPIO_PIN_1
#define LEVEL_GPIO_Port GPIOA
#define GUN_SWITCH_Pin GPIO_PIN_8
#define GUN_SWITCH_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

// General settings
#define GENERAL_TIMEOUT_MS       ((uint32_t)100)

// Defines
#define ML_IN_LTR                ((uint32_t)1000)

// EEPROM
extern I2C_HandleTypeDef         hi2c1;
#define EEPROM_I2C               (hi2c1)

// BEDUG UART
extern UART_HandleTypeDef        huart2;
#define BEDUG_UART               (huart2)

// BEDUG UART
extern UART_HandleTypeDef        huart1;
#define RS232_UART               (huart1)

// Watchdog
//extern IWDG_HandleTypeDef        hiwdg; // TODO
//#define DEVICE_IWDG              (hiwdg)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
