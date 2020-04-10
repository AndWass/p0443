
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "p0443_v2/connect.hpp"
#include "p0443_v2/start.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/tuple.hpp>
#include <p0443_v2/type_traits.hpp>
#include <tuple>

#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <utility>

namespace p0443_v2
{
namespace detail
{
template <class... Senders>
struct when_all_op
{
    using senders_storage = std::tuple<Senders...>;

    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = boost::mp11::mp_unique<boost::mp11::mp_append<
        typename p0443_v2::sender_traits<Senders>::template value_types<Tuple, Variant>...>>;

    template <template <class...> class Variant>
    using error_types = boost::mp11::mp_unique<boost::mp11::mp_append<
        typename p0443_v2::sender_traits<Senders>::template error_types<Variant>...>>;

    // static constexpr bool sends_done = (p0443_v2::sender_traits<Senders>::sends_done || ...);
    static constexpr bool sends_done = std::disjunction<
        std::bool_constant<p0443_v2::sender_traits<Senders>::sends_done>...>::value;

    senders_storage senders_;

    template <class... Tx>
    when_all_op(Tx &&... tx) : senders_(std::forward<Tx>(tx)...) {
    }

    template <class Receiver>
    struct operation_state
    {
        struct shared_state;

        struct op_receiver
        {
            std::shared_ptr<shared_state> state_;
            template <class... Values>
            void set_value(Values &&... values) {
                if (state_->next_) {
                    state_->waiting_for_--;
                    if (state_->waiting_for_ == 0) {
                        try {
                            p0443_v2::set_value(*state_->next_, std::forward<Values>(values)...);
                            state_->next_.reset();
                        }
                        catch (...) {
                            p0443_v2::set_error(*state_->next_, std::current_exception());
                        }
                    }
                }
                state_.reset();
            }

            template <class E>
            void set_error(E &&e) {
                if (state_->next_) {
                    p0443_v2::set_error(*state_->next_, std::forward<E>(e));
                    state_->next_.reset();
                }
                state_.reset();
            }

            void set_done() noexcept {
                if (state_->next_) {
                    p0443_v2::set_done(*state_->next_);
                    state_->next_.reset();
                }
                state_.reset();
            }
        };
        using operation_storage = std::tuple<p0443_v2::operation_type<Senders, op_receiver>...>;
        senders_storage senders_;

        struct shared_state
        {
            std::optional<Receiver> next_;
            int waiting_for_;
            std::optional<operation_storage> op_storage_;

            shared_state(Receiver &&recv, int waiting_for)
                : next_(std::move(recv)), waiting_for_(waiting_for) {
            }
        };

        std::shared_ptr<shared_state> state_;

        template <class Rx>
        operation_state(senders_storage &&senders, Rx &&receiver)
            : senders_(std::move(senders)),
              state_(std::make_unique<shared_state>(std::forward<Rx>(receiver),
                                                    std::tuple_size_v<senders_storage>)) {
        }

        void start() {
            auto sender_to_op = [this](auto &&... senders) {
                return operation_storage{p0443_v2::connect(std::forward<decltype(senders)>(senders),
                                                           op_receiver{state_})...};
            };

            state_->op_storage_.emplace(std::apply(sender_to_op, std::move(senders_)));
            boost::mp11::tuple_for_each(*state_->op_storage_, [](auto &op) { p0443_v2::start(op); });
        }
    };

    template <class Receiver>
    auto connect(Receiver &&receiver) {
        return operation_state<p0443_v2::remove_cvref_t<Receiver>>(
            std::move(senders_), std::forward<Receiver>(receiver));
    }
};
} // namespace detail

template <class S>
auto when_all(S &&sender) {
    return std::forward<p0443_v2::remove_cvref_t<S>>(sender);
}

template <class... Senders>
auto when_all(Senders &&... senders) {
    static_assert(sizeof...(Senders) > 0, "when_all must take at least 1 sender");
    return detail::when_all_op<p0443_v2::remove_cvref_t<Senders>...>(
        std::forward<Senders>(senders)...);
}
} // namespace p0443_v2