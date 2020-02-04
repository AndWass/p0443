
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/type_traits.hpp>

namespace p0443_v2
{
namespace detail
{
template <class S, class F>
void handle_error(S, F) = delete;

template <class OutputReceiver, class Handler>
struct handle_error_impl
{
    using output_receiver_type = p0443_v2::remove_cvref_t<OutputReceiver>;
    using handler_type = p0443_v2::remove_cvref_t<Handler>;

    output_receiver_type out_receiver_;
    handler_type handler_;

    template <class R, class Fn>
    handle_error_impl(R &&receiver, Fn &&fn)
        : out_receiver_(std::forward<R>(receiver)), handler_(std::forward<Fn>(fn)) {
    }

    template <class... Values>
    void set_value(Values &&... values) {
        try {
            p0443_v2::set_value(out_receiver_, std::forward<Values>(values)...);
        }
        catch (...) {
            this->set_error(std::current_exception());
        }
    }

    void set_done() {
        p0443_v2::set_done(out_receiver_);
    }

    template <class E>
    void set_error(E &&e) {
        p0443_v2::submit(handler_(std::forward<E>(e)), out_receiver_);
    }
};

template <class Sender, class Handler>
struct handle_error_sender
{
    using input_sender = p0443_v2::remove_cvref_t<Sender>;
    using handler_type = p0443_v2::remove_cvref_t<Handler>;

    input_sender sender_;
    handler_type handler_;

    template <class S, class H>
    handle_error_sender(S &&sender, H &&handler)
        : sender_(std::forward<S>(sender)), handler_(std::forward<H>(handler)) {
    }

    template <class Receiver>
    void submit(Receiver &&r) {
        p0443_v2::submit(sender_,
                         handle_error_impl<Receiver, Handler>(std::forward<Receiver>(r), handler_));
    }
};

class handle_error_fn
{
    template <class S, class Fn>
    using member_detector = decltype(std::declval<S>().handle_error(std::declval<Fn>()));

    template <class S, class Fn>
    using free_function_detector = decltype(handle_error(std::declval<S>(), std::declval<Fn>()));

public:
    template <class Sender, class Handler>
    auto operator()(Sender && sender, Handler && handler) const {
        using has_member = p0443_v2::is_detected<member_detector, Sender, Handler>;
        using has_free_function = p0443_v2::is_detected<free_function_detector, Sender, Handler>;

        if constexpr (has_member::value) {
            return sender.handle_error(std::forward<Handler>(handler));
        }
        else if constexpr (has_free_function::value) {
            return handle_error(std::forward<Sender>(sender), std::forward<Handler>(handler));
        }
        else {
            return handle_error_sender<Sender, Handler>(std::forward<Sender>(sender),
                                                        std::forward<Handler>(handler));
        }
    }
};
} // namespace detail
constexpr detail::handle_error_fn handle_error;
} // namespace p0443_v2