#pragma once

#include "type_traits.hpp"

#include "submit.hpp"

#include <exception>
#include <utility>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Function>
struct transform_op
{
    using sender_type = p0443_v2::remove_cvref_t<Sender>;
    using function_type = p0443_v2::remove_cvref_t<Function>;

    template <class Receiver>
    struct receiver
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;
        receiver_type receiver;
        function_type fn;

        template <class Rx, class Fn>
        receiver(Rx &&rx, Fn &&fn)
            : receiver(std::move_if_noexcept(std::forward<Rx>(rx))),
              fn(std::move_if_noexcept(std::forward<Fn>(fn))) {
        }

        template <class... Values>
        std::enable_if_t<p0443_v2::is_receiver_for_values_v<Receiver, Values...>>
        set_value(Values &&... values) {
            try {
                p0443_v2::set_value(receiver, values...);
                fn(values...);
            }
            catch (...) {
                p0443_v2::set_error(receiver, std::current_exception());
            }
        }

        void set_done() {
            p0443_v2::set_done(receiver);
        }

        template <class E>
        void set_error(E &&e) {
            p0443_v2::set_error(receiver, e);
        }
    };

    sender_type sender_;
    function_type function_;

    template <class Receiver>
    void submit(Receiver &&rx) {
        p0443_v2::submit(sender_, receiver<Receiver>(std::forward<Receiver>(rx),
                                                     std::move_if_noexcept(function_)));
    }

    template <class S, class Fn>
    transform_op(S &&sender, Fn &&fn)
        : sender_(std::forward<S>(sender)), function_(std::forward<Fn>(fn)) {
    }
};
} // namespace detail

template <class Sender, class Function>
auto transform(Sender &&sender, Function &&fn) {
    return detail::transform_op<Sender, Function>(std::forward<Sender>(sender), std::forward<Function>(fn));
}
} // namespace p0443_v2
