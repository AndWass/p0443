#pragma once

#include <exception>
#include <tuple>
#include <utility>

#include <p0443_v2/transform.hpp>

namespace p0443_v2
{
namespace detail
{
template <class... Senders>
struct sequence_op;

template <class S1, class S2>
struct sequence_op<S1, S2>
{
    template <class Receiver>
    struct sequence_receiver
    {
        p0443_v2::remove_cvref_t<S2> next_;
        p0443_v2::remove_cvref_t<Receiver> recv_;
        template <class Tx, class Rx>
        sequence_receiver(Tx &&tx, Rx &&rx)
            : next_(std::move_if_noexcept(tx)), recv_(std::move_if_noexcept(rx)) {
        }

        template <class... Values>
        void set_value(Values &&... values) {
            try {
                p0443_v2::submit((S2 &&) next_, (Receiver &&) recv_);
            }
            catch (...) {
                p0443_v2::set_error((Receiver &&) recv_, std::current_exception());
            }
        }

        void set_done() {
        }
        template <class E>
        void set_error(E &&e) {
            p0443_v2::set_error((Receiver &&) recv_, std::forward<E>(e));
        }
    };

    p0443_v2::remove_cvref_t<S1>
        first_;
    p0443_v2::remove_cvref_t<S2> second_;

    template <class T1, class T2>
    sequence_op(T1 &&t1, T2 &&t2)
        : first_(std::move_if_noexcept(t1)), second_(std::move_if_noexcept(t2)) {
    }

    template <class Receiver>
    void submit(Receiver &&rx) {
        p0443_v2::submit((S1 &&) first_, sequence_receiver<Receiver>(std::forward<S2>(second_),
                                                                     std::forward<Receiver>(rx)));
    }
};
} // namespace detail


template <class Sender>
std::decay_t<Sender> sequence(Sender &&sender) {
    return std::forward<Sender>(sender);
}

template <class S1, class... Senders>
auto sequence(S1 &&s1, Senders &&... senders) {
    using next_type = decltype(::p0443_v2::sequence(std::forward<Senders>(senders)...));
    ;
    return detail::sequence_op<S1, next_type>(
        std::forward<S1>(s1), ::p0443_v2::sequence(std::forward<Senders>(senders)...));
}
} // namespace p0443_v2