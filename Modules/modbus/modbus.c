/* Copyright © 2023 Georgy E. All rights reserved. */

#include "modbus.h"

#include "app.h"
#include "main.h"
#include "gutils.h"
#include "bmacro.h"
#include "modbus_rtu_master.h"


#define SLAVE_ADDR (1)
#define SLAVE_REG  (0)


void _request_data_sender(uint8_t* data, uint32_t len);
void _response_packet_handler(modbus_response_t* packet);
void _master_internal_error_handler(void);


util_old_timer_t timeout = {0,};
util_old_timer_t request = {0,};


void modbus_init()
{
    modbus_master_set_request_data_sender(&_request_data_sender);
    modbus_master_set_response_packet_handler(&_response_packet_handler);
    modbus_master_set_internal_error_handler(&_master_internal_error_handler);
}

void modbus_tick()
{
	if (!util_old_timer_wait(&request)) {
		return;
	}
	util_old_timer_start(&request, GENERAL_TIMEOUT_MS);

	modbus_master_read_input_registers(SLAVE_ADDR, SLAVE_REG, 1);
}

void modbus_recieve(uint8_t byte)
{
	if (!util_old_timer_wait(&timeout)) {
		modbus_master_timeout();
	}
	util_old_timer_start(&timeout, GENERAL_TIMEOUT_MS);
    modbus_master_recieve_data_byte(byte);
}

void _request_data_sender(uint8_t* data, uint32_t len)
{
	HAL_UART_Transmit(&MODBUS_UART, data, (uint16_t)len, (uint16_t)GENERAL_TIMEOUT_MS);
}

void _response_packet_handler(modbus_response_t* packet)
{
    if (packet->status != MODBUS_NO_ERROR) {
    	BEDUG_ASSERT(false, "MODBUS response error");
        app_info.level_adc = 0;
        return;
    }
    app_info.level_adc = (uint32_t)packet->response[0];
}

void _master_internal_error_handler()
{
	BEDUG_ASSERT(false, "MODBUS internal_error_handler called");
}
