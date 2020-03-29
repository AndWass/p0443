
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "p0443_v2/type_traits.hpp"
#include <optional>

#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>
#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/set_error.hpp>

namespace p0443_v2
{
template <class Sender, class Function>
struct handle_done
{
    using done_sender = decltype(std::declval<Function>()());

    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = p0443_v2::merge_sender_value_types<Tuple, Variant, Sender, done_sender>;

    template <template <class...> class Variant>
    using error_types = p0443_v2::merge_error_types<Variant, Sender, done_sender>;

    static constexpr bool sends_done = p0443_v2::sender_traits<Sender>::sends_done ||
                                       p0443_v2::sender_traits<done_sender>::sends_done;

    template<class Receiver>
    struct handle_done_receiver
    {
        Receiver receiver_;
        Function fn_;
        std::optional<p0443_v2::operation_type<done_sender&&, Receiver&&>> done_op_;

        template<class...Values>
        void set_value(Values&&...values) {
            p0443_v2::set_value(std::move(receiver_), std::forward<Values>(values)...);
        }

        void set_done() noexcept {
            done_op_.emplace(p0443_v2::connect(fn_(), std::move(receiver_)));
            p0443_v2::start(*done_op_);
        }
        template<class E>
        void set_error(E&& e) {
            p0443_v2::set_error(std::move(receiver_), std::forward<E>(e));
        }
    };

    Sender next_;
    Function fn_;

    handle_done(Sender s, Function fn) : next_(std::move(s)), fn_(std::move(fn)) {
    }

    template<class Receiver>
    auto connect(Receiver&& rx) {
        return p0443_v2::connect(std::move(next_), handle_done_receiver<p0443_v2::remove_cvref_t<Receiver>>{
            std::forward<Receiver>(rx),
            std::move(fn_)
        });
    }
};
} // namespace p0443_v2