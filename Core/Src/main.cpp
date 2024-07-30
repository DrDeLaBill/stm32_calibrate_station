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
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app.h"
#include "soul.h"
#include "bmacro.h"
#include "system.h"
#include "settings.h"
#include "gprotocol.h"
#include "STM32_adapter_BA_BLE.h"

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
	{GP_KEY_STR("status"),    {reinterpret_cast<uint8_t*>(&app_info.status),    sizeof(app_info.status)}},
	{GP_KEY_STR("sens_addr"), {reinterpret_cast<uint8_t*>(&app_info.sens_addr), sizeof(app_info.sens_addr)}},
};
gprotocol protocol(table);

utl::Timer gpTimer(GENERAL_TIMEOUT_MS);
bool gpHasPacket = false;
unsigned gpCounter = 0;
uint8_t gpVar = 0;
uint8_t gpBuf[sizeof(pack_t)] = {};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void error_loop();

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
	system_pre_load();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  if (is_error(RCC_ERROR)) {
	  system_clock_hsi_config();
  } else {
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_USART6_UART_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	set_status(LOADING);

	HAL_Delay(100);

    utl::Timer errTimer(40 * SECOND_MS);

	gprint("\n\n\n");
	printTagLog(MAIN_TAG, "The device is loading");

	SystemInfo();

	// Slave RS232 start
    HAL_UART_Receive_IT(&RS232_UART, &gpVar, 1);
    gpCounter++;

    // Gas sensor encoder
    HAL_TIM_Encoder_Start(&MD212_TIM, TIM_CHANNEL_ALL);

	app_init();

    errTimer.start();
	while (has_errors() || is_status(LOADING)) {
		soulGuard.defend();

    	if (!errTimer.wait()) {
			system_error_handler((SOUL_STATUS)get_first_error(), error_loop);
		}
	}

    system_post_load();

	printTagLog(MAIN_TAG, "The device is loaded successfully");

#ifdef DEBUG
	static unsigned last_error = get_first_error();
#endif
	set_status(WORKING);
	errTimer.start();
	while (1)
	{
		soulGuard.defend();

#ifdef DEBUG
		unsigned error = get_first_error();
		if (error && last_error != error) {
			printTagLog(MAIN_TAG, "New error: %u", error);
			last_error = error;
		}
#endif

		app_proccess();

		if (!errTimer.wait()) {
			system_error_handler((SOUL_STATUS)get_first_error(), error_loop);
		}

		if (has_errors() || is_status(LOADING)) {
			continue;
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (gpHasPacket) {
			protocol.slave_recieve(reinterpret_cast<pack_t*>(gpBuf));
			gpHasPacket = false;
		}

		errTimer.start();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == RS232_UART.Instance) {
		if (!gpTimer.wait()) {
			gpCounter = 0;
			gpHasPacket = false;
			gpTimer.start();
		}

        if (!gpHasPacket) {
        	gpBuf[gpCounter++] = gpVar;
        }

        if (gpCounter >= sizeof(pack_t)) {
        	gpHasPacket = true;
        	gpCounter = 0;
        }

        gpTimer.start();
        HAL_UART_Receive_IT(&RS232_UART, &gpVar, 1);
    } else if (huart->Instance == RS485_UART.Instance) {
    	adapter_BA_BLE_rx_cplt_callback(&ba_ble);
    } else if (huart->Instance == BEDUG_UART.Instance) {
    	asm("nop");
    } else {
    	system_error_handler(INTERNAL_ERROR, error_loop);
    }
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

void error_loop()
{
	soulGuard.defend();
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
	SOUL_STATUS err = has_errors() ? (SOUL_STATUS)get_first_error() : ERROR_HANDLER_CALLED;
	system_error_handler(err, error_loop);
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
	SOUL_STATUS err = has_errors() ? (SOUL_STATUS)get_first_error() : ASSERT_ERROR;
	system_error_handler(err, error_loop);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
