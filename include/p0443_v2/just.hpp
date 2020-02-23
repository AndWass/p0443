
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <tuple>

#include <p0443_v2/make_sender.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>

#include <p0443_v2/sender_value_type.hpp>

namespace p0443_v2
{
namespace detail
{
template <class... Values>
struct just_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<std::decay_t<Values>...>>;
    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;
    static constexpr bool sends_done = false;

    using storage_type = typename std::tuple<std::decay_t<Values>...>;
    storage_type val_;

    template <class... Vs, std::enable_if_t<std::is_constructible_v<storage_type, Vs...>> * = nullptr>
    just_sender(Vs &&... v) : val_(std::forward<Vs>(v)...) {
    }

    template <class Receiver>
    void submit(Receiver &&recv) {
        try {
            auto caller = [&recv](auto &&... values) {
                p0443_v2::set_value(std::move(recv), std::move(values)...);
            };
            std::apply(caller, std::move(val_));
        }
        catch (...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }

    template <class Receiver>
    struct just_operation
    {
        Receiver receiver_;
        storage_type values_;

        void start() {
            try {
                auto caller = [this](auto &&... values) {
                    p0443_v2::set_value(std::move(receiver_), std::forward<decltype(values)>(values)...);
                };
                std::apply(caller, std::move(values_));
            }
            catch (...) {
                p0443_v2::set_error((Receiver&&)receiver_, std::current_exception());
            }
        }
    };

    template <class Receiver>
    auto connect(Receiver &&recv) {
        return just_operation<std::decay_t<Receiver>>{std::forward<Receiver>(recv), std::move(val_)};
    }
};

template <>
struct just_sender<>
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    template <class Receiver>
    void submit(Receiver &&recv) {
        try {
            p0443_v2::set_value(recv);
        }
        catch (...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }

    template <class Receiver>
    struct just_operation
    {
        Receiver receiver_;

        void start() {
            try {
                p0443_v2::set_value((Receiver &&) receiver_);
            }
            catch (...) {
                p0443_v2::set_error((Receiver &&) receiver_, std::current_exception());
            }
        }
    };

    template <class Receiver>
    auto connect(Receiver &&recv) {
        return just_operation<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(recv)};
    }
};
} // namespace detail

constexpr auto just = [](auto &&... value) {
    return detail::just_sender<decltype(value)...>(std::forward<decltype(value)>(value)...);
};
} // namespace p0443_v2