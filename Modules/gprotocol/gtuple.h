/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GTUPLE_H_
#define _GTUPLE_H_


#include <limits>
#include <memory>
#include <cstddef>
#include <cstdint>

#include "log.h"

#include "variables.h"


template<class type_t, class callback_c = void, uint8_t LENGTH = 1>
struct gtuple
{
private:
    static_assert(!std::is_same<callback_c, void>::value, "The tuple functor must be non void");
    static_assert(LENGTH > 0, "The length must not be 0");


    static constexpr char TAG[] = "GTPL";


public:
    constexpr unsigned size() const
    {
        return sizeof(type_t);
    }

    constexpr uint8_t length() const
    {
        return LENGTH;
    }

    void details(const uint8_t index = 0)
    {
        printTagLog(TAG, "Details: index = %u; size = %d; length = %u\n", index, sizeof(type_t), length());
    }

    type_t deserialize(const uint8_t* src)
    {
    	type_t value = std::numeric_limits<type_t>::min();

        if (!src) {
            BEDUG_ASSERT(false, "The source must not be null");
        	return value;
        }

		for (unsigned i = 0; i < sizeof(type_t); i++) {
			uint8_t tmp = const_cast<uint8_t*>(src)[i];
			value <<= BITS_IN_BYTE;
			value |= tmp;
		}

		return value;
    }

    uint8_t* serialize(const uint8_t index = 0)
    {
    	static uint8_t buffer[sizeof(type_t)] = {};
    	memset(buffer, 0, sizeof(buffer));

        type_t value = static_cast<type_t>(callback_c::get(index));
        if (index >= length()) {
            BEDUG_ASSERT(false, "The index is out of range");
            details(index);
        	return buffer;
        }

		for (unsigned i = sizeof(type_t); i > 0; i--) {
			buffer[i] = static_cast<uint8_t>(value & 0xFF);
			value >>= BITS_IN_BYTE;
		}

		return buffer;
    }

    void set(type_t value, const uint8_t index = 0)
    {
    	if (value == callback_c::get(index)) {
    		return;
    	}
        callback_c::set(value, index);
    }

    void set_key(const size_t key)
    {
    	callback_c::key = key;
    }

};


#endif
