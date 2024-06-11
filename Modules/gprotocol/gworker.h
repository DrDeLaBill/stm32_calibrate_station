/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GPROTOCOL_H_
#define _GPROTOCOL_H_


#include <unordered_map>

#include "gtuple.h"
#include "gtable.h"


template<class table>
struct gworker
{
public:
    static_assert(std::is_base_of<gtable_base, table>::value, "Unresolved table class");

    static constexpr unsigned count()
    {
        return table::count();
    }

private:
    using tuple_p = typename table::tuple_p;
    using tuple_v = typename table::tuple_v;

    using tuple_t = std::unordered_map<
        size_t,
        tuple_v
    >;


    static constexpr char TAG[] = "GPCL";
    static constexpr size_t FIRST_KEY = 1;

    const size_t START_KEY;

    size_t curr_key;
    tuple_t map;


    void details(const size_t key, const uint8_t index)
    {
        printTagLog(TAG, "Details: key = %u; index = %u; max key = %u\n", key, index, count() - 1);
    }

    template<class... packs>
    void set_table(utl::simple_list_t<packs...>)
    {
        (set_tuple(utl::getType<packs>{}), ...);
        curr_key = count() - 1;
    }

    template<class pack>
    void set_tuple(utl::getType<pack> tuplePack)
    {
        using tuple_t = typename decltype(tuplePack)::TYPE;

        map.insert({curr_key, tuple_t{}});

        auto it = map.find(curr_key);
        auto lambda = [&] (auto& tuple) {
            tuple.set_key(curr_key);
        };
        std::visit(lambda, it->second);

        curr_key++;
    }

public:
    gworker(const size_t startKey = FIRST_KEY): START_KEY(startKey)
    {
        curr_key = startKey;
        set_table(tuple_p{});
    }

    void set(const size_t key, const uint8_t* value, const uint8_t index = 0)
    {
        auto it = map.find(key);
        if (it == map.end()) {
        	BEDUG_ASSERT(false, "Table not found error");
        	details(key, index);
        	return;
        }

        auto lambda = [&] (auto& tuple) {
            tuple.set(tuple.deserialize(value), index);
        };

        std::visit(lambda, it->second);
    }

    void get(const size_t key, uint8_t* dst, const uint8_t index = 0)
    {
    	auto it = map.find(key);
        if (it == map.end()) {
        	BEDUG_ASSERT(false, "Table not found error");
        	details(key, index);
        	return;
        }

        auto lambda = [&] (auto& tuple) {
            memcpy(dst, tuple.serialize(index), __min(sizeof(uint64_t), tuple.size()));
        };

        std::visit(lambda, it->second);
    }

    constexpr uint8_t length(size_t key)
    {
        auto it = map.find(key);
        if (it == map.end()) {
            BEDUG_ASSERT(false, "Table not found error");
        	details(key, 0);
            return 0;
        }

        uint8_t result = 0;
        auto lambda = [&] (const auto& tuple) {
            result = tuple.length();
        };

        std::visit(lambda, it->second);

        return result;
    }

    uint8_t index(const size_t key, const uint8_t index)
    {
    	return index > length(key) - 1 ? length(key) - 1 : index;
    }

    constexpr uint16_t min_key()
    {
        return START_KEY;
    }

    constexpr uint16_t max_key()
    {
        return START_KEY + count() - 1;
    }

    template<class type_t>
    type_t deserialize(const uint8_t* data)
    {
        if (!data) {
            BEDUG_ASSERT(false, "The data must not be null");
        	return std::numeric_limits<type_t>::min();
        }

        type_t value;
        memset(&value, 0, sizeof(value));
		for (unsigned i = 0; i < sizeof(type_t); i++) {
			uint8_t tmp = data[i];
			value <<= BITS_IN_BYTE;
			value |= tmp;
		}

		return value;
    }

    template<class type_t>
    const uint8_t* serialize(const type_t value)
    {
    	static uint8_t serialized_arr[sizeof(type_t)] = {};
    	memset(serialized_arr, 0, sizeof(serialized_arr));

    	type_t tmp = value;
		for (unsigned i = sizeof(type_t); i > 0; i--) {
			serialized_arr[i] = static_cast<uint8_t>(tmp & 0xFF);
			tmp >>= BITS_IN_BYTE;
		}

		return const_cast<const uint8_t*>(serialized_arr);
    }
};


#endif
