
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/type_traits.hpp>
#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/connect.hpp>

namespace p0443_v2
{
template<class Function>
class lazy
{
private:
    Function next_;
    using sender_type = std::invoke_result_t<Function>;
public:
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = typename p0443_v2::sender_traits<sender_type>::template value_types<Tuple, Variant>;

    template<template<class...> class Variant>
    using error_types = typename p0443_v2::sender_traits<sender_type>::template error_types<Variant>;

    static constexpr bool sends_done = p0443_v2::sender_traits<sender_type>::sends_done;

    lazy(Function factory): next_(std::move(factory)) {}

    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return p0443_v2::connect(next_(), std::forward<Receiver>(receiver));
    }
};
}