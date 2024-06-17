/* Copyright © 2023 Georgy E. All rights reserved. */

#include "level.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "main.h"
#include "utils.h"


#define LEVEL_HADC          (hadc1)
#define LEVEL_ADC_CHANNEL   (1)
#define LEVEL_ERROR         (-1)
#define LEVEL_DELAY_MS      ((uint32_t)100)
#define LEVEL_MEASURE_COUNT (10)
#define LEVEL_MEASURE_DELAY ((uint32_t)30)


typedef struct _sens_state_t {
	util_old_timer_t tim;
	uint32_t         measures_adc[LEVEL_MEASURE_COUNT];
	uint8_t          counter;
} sens_state_t;


uint32_t _get_cur_liquid_adc();


sens_state_t sens_state = { 0 };


void level_tick()
{
	if (util_old_timer_wait(&(sens_state.tim))) {
		return;
	}

	sens_state.measures_adc[sens_state.counter]   = _get_cur_liquid_adc();
	if (sens_state.counter >= __arr_len(sens_state.measures_adc)) {
		sens_state.counter = 0;
	}
	util_old_timer_start(&(sens_state.tim), LEVEL_MEASURE_DELAY);
}

uint32_t get_liquid_adc()
{
	uint32_t result = 0;
	for (unsigned i = 0; i < __arr_len(sens_state.measures_adc); i++) {
		result += sens_state.measures_adc[i];
	}
	return result / __arr_len(sens_state.measures_adc);
}

uint32_t _get_cur_liquid_adc() {
	extern ADC_HandleTypeDef hadc1;
	ADC_ChannelConfTypeDef conf = { 0 };
	conf.Channel      = LEVEL_ADC_CHANNEL;
	conf.Rank         = 1;
	conf.SamplingTime = ADC_SAMPLETIME_28CYCLES;

	if (HAL_ADC_ConfigChannel(&LEVEL_HADC, &conf) != HAL_OK) {
		return 0;
	}
	HAL_ADC_Start(&LEVEL_HADC);
	HAL_ADC_PollForConversion(&hadc1, LEVEL_DELAY_MS);
	uint32_t liquid_ADC_value = HAL_ADC_GetValue(&LEVEL_HADC);
	HAL_ADC_Stop(&LEVEL_HADC);
	return liquid_ADC_value;
}
