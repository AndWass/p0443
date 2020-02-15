
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <p0443_v2/submit.hpp>

#include <p0443_v2/sender_traits.hpp>

namespace p0443_v2
{
namespace detail
{
template<class Receiver, class Function>
struct then_receiver: Receiver
{
    Function function_;

    template<class...Values>
    void set_value(Values&&...values) {
        p0443_v2::submit(function_(values...), (Receiver&&)*this);
    }
};

template<class Sender, class Function>
struct then_sender: Sender
{
    Function function_;

    template<template<class...> class Tuple, template<class...> class Variant>
    struct value_types_extractor
    {
        template<class ST>
        using extractor = typename p0443_v2::sender_traits<ST>::template value_types<Tuple, Variant>;

        using sender_value_types = boost::mp11::mp_transform<extractor, p0443_v2::function_result_types<Variant, Function, Sender>>;

        template<class T1, class T2>
        using concat = p0443_v2::concat_value_types<Tuple, Variant, T1, T2>;

        using folded_sender_value_types = boost::mp11::mp_fold<sender_value_types, boost::mp11::mp_first<sender_value_types>, concat>;
    };

    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = typename value_types_extractor<Tuple, Variant>::folded_sender_value_types;

    template<template<class...> class Variant>
    using error_types = typename p0443_v2::sender_traits<Sender>::template error_types<Variant>;

    static constexpr bool sends_done = p0443_v2::sender_traits<Sender>::sends_done;

    template<class Receiver>
    void submit(Receiver&& recv) {
        using receiver_t = std::decay_t<Receiver>;
        p0443_v2::submit((Sender&&)*this, then_receiver<receiver_t, Function>{(Receiver&&)recv, std::move(function_)});
    }
};

struct then_cpo
{
    template<class Sender, class Function>
    auto operator()(Sender&& sender, Function&& fn) const {
        using sender_t = std::decay_t<Sender>;
        using function_t = std::decay_t<Function>;

        return then_sender<sender_t, function_t>{std::forward<Sender>(sender), std::forward<Function>(fn)};
    }
};
}
constexpr detail::then_cpo then;
}