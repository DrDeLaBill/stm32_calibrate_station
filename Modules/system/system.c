/* Copyright © 2023 Georgy E. All rights reserved. */

#include "system.h"

#include "main.h"
#include "glog.h"
#include "pump.h"
#include "hal_defs.h"


const char SYSTEM_TAG[] = "SYS";


extern RTC_HandleTypeDef hrtc;
extern ADC_HandleTypeDef hadc1;


uint16_t SYSTEM_ADC_VOLTAGE = 0;


#ifndef IS_SAME_TIME
#   define IS_SAME_TIME(TIME1, TIME2) (TIME1.Hours   == TIME2.Hours && \
                                       TIME1.Minutes == TIME2.Minutes && \
									   TIME1.Seconds == TIME2.Seconds)
#endif


void system_clock_hsi_config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
										|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
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

void system_pre_load(void)
{
	if (!MCUcheck()) {
		set_error(MCU_ERROR);
		while (1) {}
	}

	__set_bit(RCC->CR, RCC_CR_HSEON_Pos);

	unsigned counter = 0;
	while (1) {
		if (__get_bit(RCC->CR, RCC_CR_HSERDY_Pos)) {
			__reset_bit(RCC->CR, RCC_CR_HSEON_Pos);
			break;
		}

		if (counter > 0x100) {
			set_status(RCC_FAULT);
			set_error(RCC_ERROR);
			break;
		}

		counter++;
	}

#ifdef STM32F1
	uint32_t backupregister = (uint32_t)BKP_BASE;
	backupregister += (RTC_BKP_DR1 * 4U);
	SOUL_STATUS status = (SOUL_STATUS)((*(__IO uint32_t *)(backupregister)) & BKP_DR1_D);
#elif defined(STM32F4)
	HAL_PWR_EnableBkUpAccess();
	SOUL_STATUS status = (SOUL_STATUS)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
	HAL_PWR_DisableBkUpAccess();
#endif

	set_last_error(status);

    if (status == MEMORY_ERROR) {
     	set_error(MEMORY_ERROR);
    } else if (status == STACK_ERROR) {
    	set_error(STACK_ERROR);
    } else if (status == SETTINGS_LOAD_ERROR) {
    	set_error(SETTINGS_LOAD_ERROR);
    }
}

void system_post_load(void)
{
	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
	HAL_PWR_DisableBkUpAccess();

	if (get_last_error()) {
		printTagLog(SYSTEM_TAG, "Last reload error: %u", get_last_error());
	}

#ifdef STM32F1
	HAL_ADCEx_Calibration_Start(&hadc1);
#endif
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&SYSTEM_ADC_VOLTAGE, 1);
	uint64_t counter = 0;
	uint64_t count_max = HAL_RCC_GetHCLKFreq() * 10;
	util_old_timer_t timer = {0};
	util_old_timer_start(&timer, 10000);
	while (1) {
		uint32_t voltage = get_system_power();
		if (STM_MIN_VOLTAGEx10 <= voltage && voltage <= STM_MAX_VOLTAGEx10) {
			break;
		}

		if (is_error(RCC_ERROR) && counter > count_max) {
			set_error(POWER_ERROR);
			break;
		} else if (!util_old_timer_wait(&timer)) {
			set_error(POWER_ERROR);
			break;
		}

		counter++;
	}

	if (has_errors()) {
		system_error_handler(
			(get_first_error() == INTERNAL_ERROR) ?
				LOAD_ERROR :
				(SOUL_STATUS)get_first_error(),
			NULL
		);
	}
}

void system_error_handler(SOUL_STATUS error, void (*error_loop) (void))
{
	static bool called = false;
	if (called) {
		return;
	}
	called = true;

	set_error(error);

	if (!has_errors()) {
		error = INTERNAL_ERROR;
	}

	printTagLog(SYSTEM_TAG, "system_error_handler called error=%u", error);

	/* Custom events begin */
	HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VALVE2_GPIO_Port, VALVE2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
	/* Custom events end */

	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, error);
	HAL_PWR_DisableBkUpAccess();

	uint64_t counter = 0;
	uint64_t count_max = HAL_RCC_GetHCLKFreq() * 10;
	util_old_timer_t timer = {0};
	util_old_timer_start(&timer, 10000);
	while(1) {
		if (error_loop) {
			error_loop();
		}

		if (is_error(RCC_ERROR) && counter > count_max) {
			set_error(POWER_ERROR);
			break;
		} else if (!util_old_timer_wait(&timer)) {
			set_error(POWER_ERROR);
			break;
		}

		counter++;
	}

#ifdef DEBUG
	printTagLog(SYSTEM_TAG, "system reset");
	counter = 100;
	while(counter--);
#endif

	NVIC_SystemReset();
}

uint32_t get_system_power(void)
{
	if (!SYSTEM_ADC_VOLTAGE) {
		return 0;
	}
	return (STM_ADC_MAX * STM_REF_VOLTAGEx10) / SYSTEM_ADC_VOLTAGE;
}
