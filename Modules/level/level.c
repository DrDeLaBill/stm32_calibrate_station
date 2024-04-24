/* Copyright © 2023 Georgy E. All rights reserved. */

#include "level.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "main.h"
#include "utils.h"
#include "settings.h"


#define LEVEL_HADC          (hadc1)
#define LEVEL_ADC_CHANNEL   (1)
#define LEVEL_ERROR         (-1)
#define LEVEL_DELAY_MS      ((uint32_t)100)
#define LEVEL_MEASURE_COUNT (10)
#define LEVEL_MEASURE_DELAY ((uint32_t)300)


typedef struct _sens_state_t {
	util_old_timer_t tim;
	int32_t          measures_val[LEVEL_MEASURE_COUNT];
	int32_t          measures_adc[LEVEL_MEASURE_COUNT];
	uint8_t          counter;
} sens_state_t;


uint16_t _get_liquid_adc_value();
int32_t  _get_liquid_liters();
uint16_t _get_cur_liquid_adc();
int32_t  _get_liquid_liters();


extern settings_t settings;


static const char LIQUID_TAG[] = "LQID";


sens_state_t sens_state = { 0 };


void level_tick()
{
	if (util_old_timer_wait(&(sens_state.tim))) {
		return;
	}

	sens_state.measures_adc[sens_state.counter]   = _get_cur_liquid_adc();
	sens_state.measures_val[sens_state.counter++] = _get_liquid_liters();
	if (sens_state.counter >= __arr_len(sens_state.measures_val)) {
		sens_state.counter = 0;
	}
	util_old_timer_start(&(sens_state.tim), LEVEL_MEASURE_DELAY);
}

int32_t get_liquid_level()
{
	int32_t result = 0;
	for (unsigned i = 0; i < __arr_len(sens_state.measures_val); i++) {
		result += sens_state.measures_val[i];
	}
	return result / __arr_len(sens_state.measures_val);
}

uint16_t get_liquid_adc()
{
	uint16_t result = 0;
	for (unsigned i = 0; i < __arr_len(sens_state.measures_adc); i++) {
		result += sens_state.measures_adc[i];
	}
	return result / __arr_len(sens_state.measures_adc);
}

uint16_t _get_cur_liquid_adc() {
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
	uint16_t liquid_ADC_value = HAL_ADC_GetValue(&LEVEL_HADC);
	HAL_ADC_Stop(&LEVEL_HADC);
	return liquid_ADC_value;
}

int32_t _get_liquid_liters()
{
//	uint16_t liquid_ADC_value = _get_cur_liquid_adc();
//	if (liquid_ADC_value >= ADC_MAX) {
//		printTagLog(LIQUID_TAG, "error liquid tank: get liquid ADC value - value more than MAX=%d (ADC=%d)\n", MAX_ADC_VALUE, liquid_ADC_value);
//		return LEVEL_ERROR;
//	}
//
//	if (liquid_ADC_value > settings.tank_ADC_min || liquid_ADC_value < settings.tank_ADC_max) {
//		printTagLog(LIQUID_TAG, "error liquid tank: settings error - liquid_ADC_valu=%u, tank_ADC_min=%lu, tank_ADC_max=%lu\n", liquid_ADC_value, settings.tank_ADC_min, settings.tank_ADC_max);
//		return LEVEL_ERROR;
//	}
//
//	uint32_t liquid_ADC_range = __abs_dif(settings.tank_ADC_min, settings.tank_ADC_max);
//	uint32_t liquid_liters_range = __abs_dif(settings.tank_liters_max, settings.tank_liters_min) / ML_IN_LTR;
//	if (liquid_ADC_range == 0) {
//		printTagLog(LIQUID_TAG, "error liquid tank: settings error - tank_liters_range=%lu, liquid_ADC_range=%lu\n", liquid_liters_range, liquid_ADC_range);
//		return LEVEL_ERROR;
//	}
//
//	uint32_t min_in_liters = settings.tank_liters_min / ML_IN_LTR;
//	int32_t liquid_in_liters = (liquid_liters_range - ((liquid_ADC_value * liquid_liters_range) / liquid_ADC_range)) + min_in_liters;
//	if (liquid_in_liters <= 0) {
//		printTagLog(LIQUID_TAG, "error liquid tank: get liquid liters - value less or equal to zero (val=%ld)\n", liquid_in_liters);
//		return LEVEL_ERROR;
//	}
//
//	return liquid_in_liters;
}

bool is_liquid_tank_empty()
{
//	return _get_cur_liquid_adc() > settings.tank_ADC_min;
}
