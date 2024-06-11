/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GTRANSFER_H_
#define _GTRANSFER_H_


#include "log.h"
#include "gworker.h"
#include "greport.h"
#include "ginterface.h"

#ifdef USE_HAL_DRIVER
#   include "stm32f4xx_hal.h"
#else
#endif


struct gprotocol
{
private:
	static constexpr char TAG[] = "GWRK";

	using table_t = gtable<
		gtuple<uint16_t, ginterface::dv_type>,
		gtuple<uint8_t,  ginterface::sw_id>,
		gtuple<uint8_t,  ginterface::fw_id>,
		gtuple<uint32_t, ginterface::target_ml>,
		gtuple<uint32_t, ginterface::level_ml>,
		gtuple<uint32_t, ginterface::work_ml>
	>;

	gworker<table_t> worker;

	void send_report(pack_t& report)
	{
#ifdef USE_HAL_DRIVER
		HAL_UART_Transmit(&RS232_UART, reinterpret_cast<uint8_t*>(&report), sizeof(report), 100);
#else
#endif
	}

public:
	void slave_recieve(pack_t* request)
	{
		if (request->crc != pack_crc(request)) {
			return;
		}

    	pack_t response = {};
    	response.key    = 0;
    	response.index  = request->index;

    	if (request->key == PACK_GETTER_KEY) {
    		response.key   = worker.deserialize<size_t>(request->data);
    		response.index = worker.index(response.key, response.index);
    	} else {
    		response.key = request->key;
    		worker.set(request->key, request->data, request->index);
    	}

    	worker.get(response.key, response.data, response.index);

    	response.crc = pack_crc(&response);

    	send_report(response);

    	printTagLog(TAG, "Response:");
    	pack_show(&response);
	}

	void master_send(const bool send, const size_t key, const uint8_t index = 0)
	{
		pack_t request = {};
		request.key = PACK_GETTER_KEY;
		request.index = index;

		if (send) {
			request.key = key;
			worker.get(key, request.data, index);
		} else {
			memcpy(request.data, worker.serialize<size_t>(key), sizeof(key));
		}

		request.crc = pack_crc(&request);

    	send_report(request);
    	printTagLog(TAG, "Request:");
    	pack_show(&request);
	}

	void master_recieve(pack_t& response)
	{
		if (response.crc != pack_crc(&response)) {
			return;
		}

		worker.set(response.key, response.data, response.index);

    	printTagLog(TAG, "Response:");
    	pack_show(&response);
	}

};


#endif
