
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/type_traits.hpp>
#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_done.hpp>

#include <exception>
#include <utility>

#include <p0443_v2/make_receiver.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Function>
struct transform_sender
{
    using sender_type = p0443_v2::remove_cvref_t<Sender>;
    using function_type = p0443_v2::remove_cvref_t<Function>;

    template <class Receiver>
    struct receiver
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;
        receiver_type next_;
        function_type fn_;

        template <class Rx, class Fn>
        receiver(Rx &&rx, Fn &&fn) : next_(std::forward<Rx>(rx)), fn_(std::forward<Fn>(fn)) {
        }

        template <class... Values>
        void set_value(Values &&... values) {
            if constexpr (std::is_void<std::invoke_result_t<function_type, Values...>>::value) {
                try {
                    fn_(std::forward<Values>(values)...);
                    p0443_v2::set_value((receiver_type &&) next_);
                }
                catch (...) {
                    p0443_v2::set_error(next_, std::current_exception());
                }
            }
            else {
                try {
                    p0443_v2::set_value((receiver_type &&) next_,
                                        fn_(std::forward<Values>(values)...));
                }
                catch (...) {
                    p0443_v2::set_error(next_, std::current_exception());
                }
            }
        }

        void set_done() {
            p0443_v2::set_done(next_);
        }

        template <class E>
        void set_error(E &&e) {
            p0443_v2::set_error(next_, e);
        }
    };

    template<class Receiver>
    struct operation_state
    {
        using next_operation_state = decltype(p0443_v2::connect(std::declval<sender_type>(), std::declval<receiver<Receiver>>()));
        next_operation_state state_;

        operation_state(sender_type&& sender, receiver<Receiver>&& receiver): state_(p0443_v2::connect(std::move(sender), std::move(receiver))) {}

        void start() {
            p0443_v2::start(state_);
        }
    };

    sender_type sender_;
    function_type function_;

    template <template <class...> class Tuple>
    struct add_tuple
    {
        template <class T>
        using type = std::conditional_t<std::is_void_v<T>, Tuple<>, Tuple<T>>;
    };

    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = boost::mp11::mp_unique<boost::mp11::mp_transform<
        add_tuple<Tuple>::template type,
        p0443_v2::function_result_types<Variant, function_type, sender_type>>>;

    template<template<class...> class Variant>
    using error_types = p0443_v2::append_error_types<Variant, sender_type, std::exception_ptr>;

    static constexpr bool sends_done =  p0443_v2::sender_traits<sender_type>::sends_done;

    template<class Receiver>
    auto connect(Receiver &&recv) {
        return operation_state<Receiver>(std::move(sender_), receiver<Receiver>(std::forward<Receiver>(recv), function_));
    }

    template <class S, class Fn>
    transform_sender(S &&sender, Fn &&fn)
        : sender_(std::forward<S>(sender)), function_(std::forward<Fn>(fn)) {
    }
};

struct transform_fn
{
    template <class Sender, class Function>
    auto operator()(Sender && sender, Function && fn) const {
        return ::p0443_v2::detail::transform_sender<Sender, Function>(std::forward<Sender>(sender),
                                                                  std::forward<Function>(fn));
    }
};
} // namespace detail

constexpr ::p0443_v2::detail::transform_fn transform{};
} // namespace p0443_v2
