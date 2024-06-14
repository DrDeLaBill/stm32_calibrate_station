/* Copyright © 2023 Georgy E. All rights reserved. */

#include "pump.h"

#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "soul.h"
#include "utils.h"
#include "fsm_gc.h"


#define PUMP_MD212_MLSx10_PER_TICK ((int32_t)25)
#define PUMP_MD212_MLS_COUNT_MIN   ((uint32_t)10)
#define PUMP_MD212_TRIGGER_VAL_MAX ((int32_t)30000)

#define PUMP_MIN_ML     (100)
#define PUMP_TICKS_MID  ((uint32_t)0xFFFF / 2)
#define PUMP_MEAS_COUNT (10)
#define PUMP_MEAS_MS    (100)

#define PUMP_SLOW_ML_VALUE ((uint32_t)1000)


struct pump_t {
	bool             need_start;
	bool             need_stop;
	uint32_t         target_ml;
	int32_t          measure_ticks_base;
	int32_t          measure_ticks_add;
	uint32_t         measure_ml_base;
	uint32_t         measure_ml_add;
	util_old_timer_t err_timer;
} pump;


void _pump_enable();
void _pump_disable();
bool _pump_gun_ready();
void _pump_set_ticks(uint32_t ticks);
void _pump_reset_ticks();
int32_t _pump_encoder_ticks();
int32_t _pump_summary_ticks();
uint32_t _pump_encoder_ml();
uint32_t _pump_summary_ml();


void _pump_init_s();
void _pump_idle_s();
void _pump_start_s();
void _pump_work_s();
void _pump_stop_s();
void _pump_error_s();


FSM_GC_CREATE(pump_fsm)

FSM_GC_CREATE_STATE(pump_init_s,  _pump_init_s)
FSM_GC_CREATE_STATE(pump_idle_s,  _pump_idle_s)
FSM_GC_CREATE_STATE(pump_start_s, _pump_start_s)
FSM_GC_CREATE_STATE(pump_work_s,  _pump_work_s)
FSM_GC_CREATE_STATE(pump_stop_s,  _pump_stop_s)
FSM_GC_CREATE_STATE(pump_error_s, _pump_error_s)

FSM_GC_CREATE_EVENT(pump_success_e)
FSM_GC_CREATE_EVENT(pump_negative_e)
FSM_GC_CREATE_EVENT(pump_error_e)
FSM_GC_CREATE_EVENT(pump_start_e)
FSM_GC_CREATE_EVENT(pump_stop_e)

FSM_GC_CREATE_TABLE(
    pump_fsm_table,
    { &pump_init_s,   &pump_success_e,  &pump_stop_s  },
    { &pump_idle_s,   &pump_start_e,    &pump_start_s },
    { &pump_idle_s,   &pump_stop_e,     &pump_stop_s  },
    { &pump_idle_s,   &pump_error_e,    &pump_error_s },
    { &pump_start_s,  &pump_success_e,  &pump_work_s  },
    { &pump_start_s,  &pump_negative_e, &pump_idle_s  },
    { &pump_work_s,   &pump_stop_e,     &pump_stop_s  },
    { &pump_work_s,   &pump_error_e,    &pump_error_s },
    { &pump_stop_s,   &pump_success_e,  &pump_idle_s  },
    { &pump_error_s,  &pump_success_e,  &pump_idle_s  }
)


void pump_init()
{
	fsm_gc_init(&pump_fsm, pump_fsm_table, __arr_len(pump_fsm_table));
}

void pump_proccess()
{
    fsm_gc_proccess(&pump_fsm);
}

void set_pump_target(uint32_t target_ml)
{
	pump.target_ml = target_ml;
}

void pump_start()
{
	pump.need_start = true;
}

void pump_stop()
{
	pump.need_start = false;
	pump.need_stop  = true;
}

void _pump_enable()
{
    HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(VALVE2_GPIO_Port, VALVE2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);
}

void _pump_disable()
{
    HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(VALVE2_GPIO_Port, VALVE2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
}

bool _pump_gun_ready()
{
	return HAL_GPIO_ReadPin(GUN_SWITCH_GPIO_Port, GUN_SWITCH_Pin);
}

int32_t _pump_encoder_ticks()
{
    int32_t value = (int32_t)__HAL_TIM_GET_COUNTER(&MD212_TIM);
    int32_t result = 0;
    if (value >= (int32_t)(PUMP_TICKS_MID)) {
        result = (int32_t)(value - (int32_t)PUMP_TICKS_MID);
    } else {
        result = -((int32_t)((int32_t)PUMP_TICKS_MID - value));
    }
    if (__abs(result) <= (int32_t)PUMP_MD212_MLS_COUNT_MIN) {
        return 0;
    }
    return result;
}

int32_t _pump_summary_ticks()
{
	return pump.measure_ticks_base + pump.measure_ticks_add;
}

void _pump_set_ticks(uint32_t ticks)
{
	__HAL_TIM_SET_COUNTER(&MD212_TIM, (uint32_t)ticks + PUMP_TICKS_MID);
}

void _pump_reset_ticks()
{
    __HAL_TIM_SET_COUNTER(&MD212_TIM, PUMP_TICKS_MID);
}

uint32_t _pump_encoder_ml()
{
    return (uint32_t)((_pump_encoder_ticks() * PUMP_MD212_MLSx10_PER_TICK) / 10);
}

uint32_t _pump_summary_ml()
{
    return pump.measure_ml_base + pump.measure_ml_add;
}

void _pump_init_s()
{
	memset((uint8_t*)&pump, 0, sizeof(pump));
	fsm_gc_push_event(&pump_fsm, &pump_success_e);
}

void _pump_idle_s()
{
	if (pump.need_start) {
		fsm_gc_push_event(&pump_fsm, &pump_start_e);
	}

	if (pump.need_stop) {
		fsm_gc_push_event(&pump_fsm, &pump_stop_e);
	}

	if (is_error(PUMP_ERROR)) {
		fsm_gc_push_event(&pump_fsm, &pump_error_e);
	}
}

void _pump_start_s()
{
	if (pump.target_ml < PUMP_MIN_ML) {
		fsm_gc_push_event(&pump_fsm, &pump_negative_e);
		pump.target_ml = 0;
		return;
	}

	if (!_pump_gun_ready()) {
		return;
	}

	memset((uint8_t*)&pump, 0, sizeof(pump));

	_pump_reset_ticks();

	_pump_enable();

	HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);
	fsm_gc_push_event(&pump_fsm, &pump_success_e);
}

void _pump_work_s()
{
	if (pump.need_stop) {
		fsm_gc_push_event(&pump_fsm, &pump_stop_e);
	}

    if (!_pump_gun_ready()) {
		fsm_gc_push_event(&pump_fsm, &pump_stop_e);
		return;
    }

    uint32_t curr_ml = _pump_encoder_ml();
    if (_pump_encoder_ticks() < pump.measure_ticks_add) {
    	_pump_set_ticks((uint32_t)pump.measure_ticks_add);
    }

    if (__abs_dif(curr_ml, _pump_summary_ml()) == 0) {
        return;
    }

    if (_pump_encoder_ticks() >= PUMP_MD212_TRIGGER_VAL_MAX) {
        _pump_reset_ticks();
        pump.measure_ml_base += pump.measure_ml_add;
        pump.measure_ml_add = 0;
        pump.measure_ticks_base += pump.measure_ticks_add;
        pump.measure_ticks_add = 0;
    }

    if (pump.measure_ml_add == _pump_encoder_ml()) {
        return;
    }

    pump.measure_ml_add = _pump_encoder_ml();
    pump.measure_ticks_add = _pump_encoder_ticks();


    uint32_t fast_target_ml =
        pump.target_ml > PUMP_SLOW_ML_VALUE ?
		pump.target_ml - PUMP_SLOW_ML_VALUE :
        0;
    if (_pump_summary_ml() >= fast_target_ml) {
        HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_RESET);
    }

    if (_pump_summary_ml() >= pump.target_ml) {
		fsm_gc_push_event(&pump_fsm, &pump_stop_e);
    }
}

void _pump_stop_s()
{
	_pump_disable();

	fsm_gc_push_event(&pump_fsm, &pump_success_e);
}

void _pump_error_s()
{
	_pump_disable();

	if (!is_error(PUMP_ERROR)) {
		fsm_gc_push_event(&pump_fsm, &pump_success_e);
	}
}
