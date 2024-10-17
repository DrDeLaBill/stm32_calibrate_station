/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GTRANSFER_H_
#define _GTRANSFER_H_


#include <cstring>
#include <unordered_map>

#include "hal_defs.h"

#include "glog.h"
#include "gutils.h"
#include "gtuple.h"
#include "greport.h"


#ifdef USE_HAL_DRIVER
#   include "main.h"
#elif defined(ARDUINO)
#   include <Arduino.h>
#else
#   error Please check your platform
#endif


#ifdef DEBUG
#   if GPROTOCOL_BEDUG_LEVEL > 0 || defined(GPROTOCOL_BEDUG)
#       define GPROTOCOL_BEDUG_L1
#   endif
#   if GPROTOCOL_BEDUG_LEVEL > 1
#       define GPROTOCOL_BEDUG_L2
#   endif
#endif


#ifndef GP_KEY_NAME
#   define GP_KEY_NAME(VAR) gprotocol::str_hash((char*)__STR_DEF2__(VAR))
#endif

#ifndef GP_KEY_STR
#   define GP_KEY_STR(STR) gprotocol::str_hash((char*)STR)
#endif

#ifndef GP_KEY_SIZE
#   define GP_KEY_SIZE     (32)
#endif


struct gprotocol
{
private:
	static constexpr char TAG[] = "GPTL";

#ifdef GPROTOCOL_BEDUG_L1
	static std::unordered_map<uint32_t, char*> debug_table;
#endif

	using type_t = uint64_t;

#ifdef GPROTOCOL_BEDUG_L1
	bool is_master_last_get;
#endif

public:
	static uint32_t str_hash(const char* data)
	{
		uint32_t hash = 0;

		for(unsigned i = 0; i < strlen(data); i++) {
			hash += data[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}

		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);

#ifdef GPROTOCOL_BEDUG_L1
		if (debug_table.find(hash) == debug_table.end()) {
			char* name = new char[strlen(data) + 1];
			memset(name, 0, strlen(data) + 1);
			memcpy(name, data, strlen(data));
			debug_table.insert(std::make_pair(hash, name));
		}
#endif

		return hash;
	}

private:
	std::unordered_map<uint32_t, gtuple>& table;

#ifdef GPROTOCOL_BEDUG_L1
	void pack_show_hum(const uint32_t key, const uint8_t index, const uint8_t* data, const bool is_get)
	{
    	auto it = debug_table.find(key);
    	BEDUG_ASSERT(it != debug_table.end(), "Debug table not found error");
    	if (it != debug_table.end()) {
        	printPretty(
    			"%s[%03u] %s: ",
				it->second,
				index,
				(is_get ? "get" : "set")
    		);
			for (unsigned i = 0; i < GPROTOCOL_DATA_SIZE; i++) {
				gprint("%03u ", data[i]);
			}
            gprint("\n");
    	} else {
    		printPretty("key: %lu\n", key);
    	}
	}
#endif

	void send_report(pack_t* const report)
	{
#ifdef USE_HAL_DRIVER
		HAL_UART_Transmit(&RS232_UART, reinterpret_cast<uint8_t*>(report), sizeof(pack_t), 100);
#elif defined(ARDUINO)
		Serial2.write(reinterpret_cast<uint8_t*>(report), sizeof(pack_t));
#endif
	}

	uint8_t index(const uint32_t key, const uint8_t index = 0)
	{
		auto it = table.find(key);
		if (it == table.end()) {
            BEDUG_ASSERT(false, "Table not found error");
            return 0;
		}
		return index;
	}

    static type_t deserialize(const uint8_t* src, const unsigned size)
    {
    	type_t value = std::numeric_limits<type_t>::min();

        if (!src) {
            BEDUG_ASSERT(false, "The source must not be null");
        	return value;
        }

		for (unsigned i = 0; i < size; i++) {
			uint8_t tmp = src[i];
			value <<= BITS_IN_BYTE;
			value |= tmp;
		}

		return value;
    }

    static uint8_t* serialize(const type_t value, const unsigned size)
    {
        static uint8_t serialized[sizeof(type_t)];
    	memset(serialized, 0, sizeof(serialized));

    	type_t tmp = value;
		for (unsigned i = size; i > 0; i--) {
			serialized[i - 1] = static_cast<uint8_t>(tmp & 0xFF);
			tmp >>= BITS_IN_BYTE;
		}

		return serialized;
    }

    void set_from_serialized(const uint32_t key, uint8_t* const value, const uint8_t index = 0)
    {
    	auto it = table.find(key);
    	if (it == table.end()) {
        	BEDUG_ASSERT(false, "Table not found error");
        	return;
    	}

		type_t tmp = deserialize(value, it->second.item_size());
		it->second.set(reinterpret_cast<uint8_t*>(&tmp), index);
    }

    void get_serialized(const uint32_t key, uint8_t* dst, const uint8_t index = 0)
    {
    	auto it = table.find(key);
    	if (it == table.end()) {
        	BEDUG_ASSERT(false, "Table not found error");
        	return;
    	}

		type_t tmp = 0;
		it->second.get(reinterpret_cast<uint8_t*>(&tmp), index);
		memcpy(dst, serialize(tmp, it->second.item_size()), it->second.item_size());
    }


public:
	gprotocol(std::unordered_map<uint32_t, gtuple>& table):
#ifdef GPROTOCOL_BEDUG_L1
		is_master_last_get(false),
#endif
		table(table) {}

	void slave_recieve(pack_t* request)
	{
		if (request->crc != pack_crc(request)) {
			return;
		}

    	pack_t response = {};
    	response.key    = 0;
    	response.index  = request->index;

    	if (request->key == PACK_GETTER_KEY) {
    		response.key   = static_cast<uint32_t>(deserialize(request->data, sizeof(response.key)));
    		response.index = index(response.key, request->index);
    	} else {
    		response.key = request->key;
    		set_from_serialized(request->key, request->data, request->index);
    	}

#ifdef GPROTOCOL_BEDUG_L1
    	printTagLog(TAG, "Request:");
    	pack_show_hum(response.key, request->index, request->data, request->key == PACK_GETTER_KEY);
#endif
#ifdef GPROTOCOL_BEDUG_L2
    	pack_show(request);
#endif

    	get_serialized(response.key, response.data, response.index);

    	response.crc = pack_crc(&response);

    	send_report(&response);

#ifdef GPROTOCOL_BEDUG_L1
    	printTagLog(TAG, "Response:");
    	pack_show_hum(response.key, response.index, response.data, request->key == PACK_GETTER_KEY);
#endif
#ifdef GPROTOCOL_BEDUG_L2
    	pack_show(&response);
#endif
	}

	void master_send(const bool send, const uint32_t key, const uint8_t index = 0)
	{
#ifdef GPROTOCOL_BEDUG_L1
		is_master_last_get = !send;
#endif

		pack_t request = {};
		request.key = PACK_GETTER_KEY;
		request.index = index;

		if (send) {
			request.key = key;
			get_serialized(key, request.data, index);
		} else {
			memcpy(request.data, serialize(static_cast<type_t>(key), sizeof(key)), sizeof(key));
		}

		request.crc = pack_crc(&request);

    	send_report(&request);

#ifdef GPROTOCOL_BEDUG_L1
    	printTagLog(TAG, "Request:");
    	pack_show_hum(key, index, request.data, !send);
#endif
#ifdef GPROTOCOL_BEDUG_L2
    	pack_show(&request);
#endif
	}

	bool master_recieve(pack_t* response)
	{
		if (response->crc != pack_crc(response)) {
			return false;
		}

		set_from_serialized(response->key, response->data, response->index);

#ifdef GPROTOCOL_BEDUG_L1
    	printTagLog(TAG, "Response:");
    	pack_show_hum(response->key, response->index, response->data, is_master_last_get);
#endif
#ifdef GPROTOCOL_BEDUG_L2
    	pack_show(response);
#endif

    	return true;
	}

};


#endif
