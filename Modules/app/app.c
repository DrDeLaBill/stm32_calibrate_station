/* Copyright © 2023 Georgy E. All rights reserved. */

#include "app.h"

#include "main.h"
#include "soul.h"
#include "pump.h"
#include "fsm_gc.h"
#include "modbus.h"






void _update_info();

void _app_init_s();
void _app_idle_s();
void _app_start_s();
void _app_count_s();
void _app_stop_s();
void _app_error_s();


FSM_GC_CREATE(app_fsm)

FSM_GC_CREATE_STATE(app_init_s,  _app_init_s)
FSM_GC_CREATE_STATE(app_idle_s,  _app_idle_s)
FSM_GC_CREATE_STATE(app_start_s, _app_start_s)
FSM_GC_CREATE_STATE(app_count_s, _app_count_s)
FSM_GC_CREATE_STATE(app_stop_s,  _app_stop_s)
FSM_GC_CREATE_STATE(app_error_s, _app_error_s)

FSM_GC_CREATE_EVENT(app_success_e)
FSM_GC_CREATE_EVENT(app_negative_e)
FSM_GC_CREATE_EVENT(app_error_e)
FSM_GC_CREATE_EVENT(app_start_e)
FSM_GC_CREATE_EVENT(app_stop_e)

FSM_GC_CREATE_TABLE(
    app_fsm_table,
    { &app_init_s,   &app_success_e, &app_idle_s  },
    { &app_idle_s,   &app_start_e,   &app_start_s },
    { &app_idle_s,   &app_error_e,   &app_error_s },
    { &app_start_s,  &app_success_e, &app_count_s },
    { &app_start_s,  &app_error_e,   &app_error_s },
    { &app_count_s,  &app_stop_e,    &app_stop_s  },
    { &app_count_s,  &app_error_e,   &app_error_s },
    { &app_stop_s,   &app_success_e, &app_idle_s  },
    { &app_stop_s,   &app_error_e,   &app_error_s },
    { &app_error_s,  &app_success_e, &app_idle_s  }
)

app_info_t app_info = {0};

Adapter_BA_BLE ba_ble;


void app_init()
{
	adapter_BA_BLE_initialization(&ba_ble, &MODBUS_UART);

    modbus_init();

	pump_init();

	fsm_gc_init(&app_fsm, app_fsm_table, __arr_len(app_fsm_table));
}

uint32_t app_get_level() {
	static uint32_t get_level = 0;

	adapter_BA_BLE_while(&ba_ble);

	switch(ba_ble.status) {
		case ADAPTER_BA_BLE_ERROR:
			set_error(SENSOR_ERROR);
			break;
		case ADAPTER_BA_BLE_OK:
			reset_error(SENSOR_ERROR);
			get_level = (uint32_t)ba_ble.data_sensor.level;
			adapter_BA_BLE_single_data_read(&ba_ble, NETWORK_ADDRESS_SENSOR);
			break;
		case ADAPTER_BA_BLE_INITIALIZATION:
		case ADAPTER_BA_BLE_EXPECTATION:
		default:
			break;
	};

	return get_level;
}

void app_proccess()
{
	modbus_tick();

	pump_proccess();

	_update_info();

	app_info.level_adc = app_get_level();

    fsm_gc_proccess(&app_fsm);
}

void _update_info()
{
	if (has_errors()) {
		app_info.status = get_first_error();
	} else if (is_status(LOADING)) {
		app_info.status = LOADING;
	} else if (is_status(WORKING)) {
		app_info.status = WORKING;
	} else {
		app_info.status = 0;
	}
}

void _app_init_s()
{
	fsm_gc_push_event(&app_fsm, &app_success_e);
}

void _app_idle_s()
{
	if (app_info.start && app_info.target_ml) {
		app_info.result_ml = 0;
		fsm_gc_push_event(&app_fsm, &app_start_e);
	}
	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
	}
}

void _app_start_s()
{
	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
	} else {
		set_pump_target(app_info.target_ml);
		pump_start();
		set_status(WORKING);
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}

void _app_count_s()
{
	app_info.result_ml = pump_count_ml();

	if (pump_stopped()) {
		reset_status(WORKING);
		fsm_gc_push_event(&app_fsm, &app_stop_e);
		return;
	}
	if (!app_info.start) {
		pump_stop();
		return;
	}
	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
		return;
	}
}

void _app_stop_s()
{
	app_info.start = 0;
	app_info.result_ml = pump_count_ml();

	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
	} else {
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}

void _app_error_s()
{
	pump_stop();
	reset_status(WORKING);

	if (!has_errors()) {
		fsm_gc_clear(&app_fsm);
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}
