/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _GTABLE_H_
#define _GTABLE_H_


#include <memory>
#include <cstdint>
#include <variant>

#include "TypeListService.h"


struct gtable_base {};

template<class... table>
struct gtable : gtable_base
{
    static_assert(
        !utl::empty(typename utl::typelist_t<table...>::RESULT{}),
        "Empty tuples table"
    );

    static_assert(
        std::is_same_v<
            typename utl::variant_factory<utl::typelist_t<table...>>::VARIANT,
            typename utl::variant_factory<utl::removed_duplicates_t<table...>>::VARIANT
            >,
        "Repeated tuples"
    );

    using tuple_p = utl::simple_list_t<table...>;
    using tuple_v = std::variant<table...>;

    static constexpr unsigned count()
    {
        return sizeof...(table);
    }

};


#endif
