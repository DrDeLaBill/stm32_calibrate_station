/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GINTERFACE_H_
#define _GINTERFACE_H_


#include <cstdint>
#include <cstddef>


struct ginterface
{
	struct dv_type
	{
		static size_t key;
		static uint16_t get(const uint8_t index = 0);
		static void set(const uint16_t value, const uint8_t index = 0);
	};
	struct sw_id
	{
		static size_t key;
		static uint8_t get(const uint8_t index = 0);
		static void set(const uint8_t value, const uint8_t index = 0);
	};
	struct fw_id
	{
		static size_t key;
		static uint8_t get(const uint8_t index = 0);
		static void set(const uint8_t value, const uint8_t index = 0);
	};
	struct target_ml
	{
		static size_t key;
		static uint32_t get(const uint8_t index = 0);
		static void set(const uint32_t value, const uint8_t index = 0);
	};
	struct level_ml
	{
		static size_t key;
		static uint32_t get(const uint8_t index = 0);
		static void set(const uint32_t value, const uint8_t index = 0);
	};
	struct work_ml
	{
		static size_t key;
		static uint32_t get(const uint8_t index = 0);
		static void set(const uint32_t value, const uint8_t index = 0);
	};
};


#endif
