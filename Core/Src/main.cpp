/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app.h"
#include "soul.h"
#include "level.h"
#include "bmacro.h"

#include "gprotocol.h"

#include "Timer.h"
#include "SoulGuard.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static constexpr char MAIN_TAG[] = "MAIN";

extern settings_t settings;

SoulGuard<
	RestartWatchdog,
	StackWatchdog,
	SettingsWatchdog,
	PowerWatchdog
> soulGuard;


std::unordered_map<uint32_t, gtuple> table = {
	{GP_KEY_STR("dv_type"),   {reinterpret_cast<uint8_t*>(&settings.dv_type),   sizeof(settings.dv_type)}},
	{GP_KEY_STR("sw_id"),     {reinterpret_cast<uint8_t*>(&settings.sw_id),     sizeof(settings.sw_id)}},
	{GP_KEY_STR("fw_id"),     {reinterpret_cast<uint8_t*>(&settings.fw_id),     sizeof(settings.fw_id)}},
	{GP_KEY_STR("start"),     {reinterpret_cast<uint8_t*>(&app_info.start),     sizeof(app_info.start)}},
	{GP_KEY_STR("target_ml"), {reinterpret_cast<uint8_t*>(&app_info.target_ml), sizeof(app_info.target_ml)}},
	{GP_KEY_STR("level_adc"), {reinterpret_cast<uint8_t*>(&app_info.level_adc), sizeof(app_info.level_adc)}},
	{GP_KEY_STR("result_ml"), {reinterpret_cast<uint8_t*>(&app_info.result_ml), sizeof(app_info.result_ml)}},
	{GP_KEY_STR("status"),    {reinterpret_cast<uint8_t*>(&app_info.status),    sizeof(app_info.status)}}
};
gprotocol protocol(table);
utl::Timer pTimer(100);
bool pHasPacket = false;
unsigned gprotocol_counter = 0;
uint8_t gprotocol_buf[sizeof(pack_t)] = {};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void update_app_info();
void system_error_handler();

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	HAL_Delay(100);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	set_status(WAIT_LOAD);

	gprint("\n\n\n");
	printTagLog(MAIN_TAG, "The device is loading");

	while (is_status(WAIT_LOAD)) soulGuard.defend();

	app_init();

    HAL_UART_Receive_IT(&RS232_UART, (uint8_t*)&gprotocol_buf[gprotocol_counter++], 1);

	printTagLog(MAIN_TAG, "The device is loaded successfully");

	reset_status(WAIT_LOAD);

	pTimer.start();
	while (1)
	{
		soulGuard.defend();

		app_proccess();

		if (has_errors()) {
			continue;
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		level_tick();

		if (pHasPacket) {
			protocol.slave_recieve(reinterpret_cast<pack_t*>(gprotocol_buf));
			pHasPacket = false;
		}
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void update_app_info()
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == RS232_UART.Instance) {
		if (!pTimer.wait()) {
			gprotocol_counter = 0;
			pHasPacket = false;
			pTimer.start();
		} else {
			gprotocol_counter++;
		}
        if (gprotocol_counter >= sizeof(pack_t)) {
        	pHasPacket = true;
        	gprotocol_counter = 0;
        }
        pTimer.start();
        HAL_UART_Receive_IT(&RS232_UART, (uint8_t*)&gprotocol_buf[gprotocol_counter], 1);
    } else {
    	system_error_handler();
    }
}


void system_error_handler()
{
	utl::Timer timer(30000);
	timer.start();
	while (timer.wait());
	NVIC_SystemReset();
}

int _write(int, uint8_t *ptr, int len) {
	(void)ptr;
	(void)len;
#ifdef DEBUG
    HAL_UART_Transmit(&BEDUG_UART, (uint8_t *)ptr, static_cast<uint16_t>(len), GENERAL_TIMEOUT_MS);
    for (int DataIdx = 0; DataIdx < len; DataIdx++) {
        ITM_SendChar(*ptr++);
    }
    return len;
#endif
    return 0;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    b_assert(__FILE__, __LINE__, "The error handler has been called");
	set_error(ERROR_HANDLER_CALLED);
	system_error_handler();
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
	b_assert((char*)file, line, "Wrong parameters value");
	set_error(ASSERT_ERROR);
	system_error_handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
