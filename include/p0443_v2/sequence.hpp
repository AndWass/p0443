
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exception>
#include <tuple>
#include <utility>
#include <variant>

#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/sender_value_type.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>
#include <p0443_v2/submit.hpp>

namespace p0443_v2
{
namespace detail
{
template <class... Senders>
struct sequence_sender;

template <class S1, class S2>
struct sequence_sender<S1, S2>
{
    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = typename p0443_v2::sender_traits<S2>::template value_types<Tuple, Variant>;

    template <template <class...> class Variant>
    using error_types = p0443_v2::append_error_types<Variant, S2, std::exception_ptr>;

    static constexpr bool sends_done = p0443_v2::sender_traits<S2>::sends_done;

    template <class Receiver>
    struct operation_state
    {
        struct second_receiver
        {
            operation_state *state_;

            second_receiver(operation_state *state) : state_(state) {
            }

            template <class... Values>
            void set_value(Values &&... values) {
                p0443_v2::set_value(std::move(state_->receiver_), std::forward<Values>(values)...);
            }

            void set_done() {
                p0443_v2::set_done(std::move(state_->receiver_));
            }

            template <class E>
            void set_error(E &&error) {
                p0443_v2::set_error(std::move(state_->receiver_), std::forward<E>(error));
            }
        };
        struct first_receiver
        {
            operation_state *state_;

            first_receiver(operation_state *state) : state_(state) {
            }

            template <class... Values>
            void set_value(Values &&...) {
                // this will be destroyed below! Only use local variables!!!
                auto *state = state_;
                try {
                    auto &ref = state->state_.template emplace<1>(
                        p0443_v2::connect(state_->second_sender_, second_receiver(state)));
                    auto index = state->state_.index();
                    auto valueless = state->state_.valueless_by_exception();
                    p0443_v2::start(ref);
                }
                catch (...) {
                    p0443_v2::set_error(std::move(state->receiver_), std::current_exception());
                }
            }

            void set_done() {
                p0443_v2::set_done(std::move(state_->receiver_));
            }

            template <class E>
            void set_error(E &&error) {
                p0443_v2::set_error(std::move(state_->receiver_), std::forward<E>(error));
            }
        };

        operation_state(S1 &&first, S2 &&second, Receiver receiver)
            : first_sender_(std::move(first)), second_sender_(std::move(second)),
              receiver_(std::move(receiver)), state_(std::in_place_index<2>) {
        }

        void start() {
            auto &ref = state_.template emplace<0>(
                p0443_v2::connect(std::move(first_sender_), first_receiver(this)));
            p0443_v2::start(ref);
        }

        using first_connect_type = p0443_v2::remove_cvref_t<decltype(
            p0443_v2::connect(std::declval<S1 &&>(), std::declval<first_receiver &&>()))>;
        using second_connect_type = p0443_v2::remove_cvref_t<decltype(
            p0443_v2::connect(std::declval<S2 &>(), std::declval<second_receiver &&>()))>;
        p0443_v2::remove_cvref_t<S1> first_sender_;
        p0443_v2::remove_cvref_t<S2> second_sender_;
        Receiver receiver_;
        std::variant<first_connect_type, second_connect_type, std::monostate> state_;
    };

    p0443_v2::remove_cvref_t<S1> first_;
    p0443_v2::remove_cvref_t<S2> second_;

    template <class T1, class T2>
    sequence_sender(T1 &&t1, T2 &&t2)
        : first_(std::move_if_noexcept(t1)), second_(std::move_if_noexcept(t2)) {
    }

    template <class Receiver>
    auto connect(Receiver &&receiver) {
        return operation_state<p0443_v2::remove_cvref_t<Receiver>>(
            std::move(first_), std::move(second_), std::move(receiver));
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
    return detail::sequence_sender<p0443_v2::remove_cvref_t<S1>,
                                   p0443_v2::remove_cvref_t<next_type>>(
        std::forward<S1>(s1), ::p0443_v2::sequence(std::forward<Senders>(senders)...));
}
} // namespace p0443_v2