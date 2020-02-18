
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if __has_include(<coroutine>)
#include <coroutine>
namespace p0443_v2
{
namespace stdcoro = std;
}
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
namespace p0443_v2
{
namespace stdcoro = std::experimental;
}
#else
#error "Support for coroutines not detected"
#endif

#include <p0443_v2/connect.hpp>
#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/start.hpp>
#include <p0443_v2/type_traits.hpp>

#include <atomic>
#include <exception>
#include <optional>
#include <tuple>
#include <variant>

namespace p0443_v2
{
struct await_done_result : public std::exception
{
};
namespace detail
{
template <class Sender>
struct await_sender
{
    using value_storage =
        typename sender_traits<Sender>::template flattened_value_type<std::tuple, std::variant>;
    using error_storage = typename sender_traits<Sender>::template error_types<std::variant>;

    template<class ValueStorage>
    struct awaitable_base
    {
        struct awaitable_receiver
        {
            awaitable_base *state_;

            awaitable_receiver(awaitable_base *state) : state_(state) {
            }

            template <class... Values>
            void set_value(Values &&... values) {
                state_->values_.template emplace<1>(std::forward<Values>(values)...);
                state_->try_resume_continuation();
            }

            void set_done() {
                state_->values_.template emplace<0>();
                state_->try_resume_continuation();
            }

            template <class E>
            void set_error(E &&e) {
                state_->values_.template emplace<2>(std::forward<E>(e));
                state_->try_resume_continuation();
            }
        };

        Sender sender_;
        std::variant<std::monostate, ValueStorage, error_storage> values_;
        std::optional<decltype(
            p0443_v2::connect(std::declval<Sender &&>(), std::declval<awaitable_receiver &&>()))>
            stored_operation_;
        stdcoro::coroutine_handle<> continuation_;
        std::atomic_bool state_{false};

        awaitable_base(Sender &&sender) : sender_(std::move(sender)) {
        }

        void try_resume_continuation() {
            auto prev = state_.exchange(true);
            if (prev == false) {
                // Continuation not set yet, do nothing
            }
            else {
                // Continuation has been set, resume using continuation
                continuation_.resume();
            }
        }

        bool try_set_continuation(stdcoro::coroutine_handle<> continuation) {
            continuation_ = continuation;
            return !state_.exchange(true, std::memory_order_acq_rel);
        }

        bool await_ready() {
            return false;
        }

        struct error_visitor
        {
            void operator()(std::exception_ptr e) {
                std::rethrow_exception(e);
            }
            template <class T, std::enable_if_t<!std::is_same_v<T, std::exception_ptr>> * = nullptr>
            void operator()(T e) {
                throw e;
            }
        };

        bool await_suspend(stdcoro::coroutine_handle<> suspended_coro) {
            stored_operation_ = p0443_v2::connect(std::move(sender_), awaitable_receiver(this));
            p0443_v2::start(*stored_operation_);
            return try_set_continuation(suspended_coro);
        }
    };

    template<class Sender, class = typename p0443_v2::sender_traits<Sender>::template value_types<std::tuple, std::variant>>
    struct awaitable: awaitable_base<value_storage>
    {
        using awaitable_base::awaitable_base;
        value_storage await_resume() {
            if (values_.index() == 0) {
                throw await_done_result();
            }
            else if (values_.index() == 2) {
                std::visit(error_visitor{}, std::get<2>(values_));
            }
            return std::get<1>(values_);
        }
    };

    template<class Sender>
    struct awaitable<Sender, std::variant<std::tuple<>>>: awaitable_base<std::monostate>
    {
        using awaitable_base::awaitable_base;
        void await_resume() {
            if (values_.index() == 0) {
                throw await_done_result();
            }
            else if (values_.index() == 2) {
                std::visit(error_visitor{}, std::get<2>(values_));
            }
        }
    };


    Sender sender_;

    await_sender(Sender &&sender) : sender_(std::move(sender)) {
    }

    auto operator co_await() {
        return awaitable<Sender>(std::move(sender_));
    }
};
} // namespace detail
inline constexpr struct await_sender_cpo
{
    template <class Sender>
    auto operator()(Sender &&sender) const {
        return detail::await_sender<p0443_v2::remove_cvref_t<Sender>>(std::forward<Sender>(sender));
    }
} await_sender;
} // namespace p0443_v2