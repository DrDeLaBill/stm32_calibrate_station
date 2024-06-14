/* Copyright © 2023 Georgy E. All rights reserved. */

#include "app.h"

#include "main.h"
#include "soul.h"
#include "pump.h"
#include "fsm_gc.h"


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


void app_init()
{
	pump_init();

	fsm_gc_init(&app_fsm, app_fsm_table, __arr_len(app_fsm_table));
}

void app_proccess()
{
	pump_proccess();

    fsm_gc_proccess(&app_fsm);
}


void _app_init_s()
{
	fsm_gc_push_event(&app_fsm, &app_success_e);
}

void _app_idle_s()
{
	if (app_info.start && app_info.target_ml) {
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
		pump_start();
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}

void _app_count_s()
{
	if (app_info.result_ml >= app_info.target_ml) {
		fsm_gc_push_event(&app_fsm, &app_stop_e);
	}
	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
	}
}

void _app_stop_s()
{
	if (has_errors()) {
		fsm_gc_push_event(&app_fsm, &app_error_e);
	} else {
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}

void _app_error_s()
{
	pump_stop();
	if (!has_errors()) {
		fsm_gc_clear(&app_fsm);
		fsm_gc_push_event(&app_fsm, &app_success_e);
	}
}
