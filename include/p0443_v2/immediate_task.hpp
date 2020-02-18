
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/await_sender.hpp>

namespace p0443_v2
{
struct immediate_task
{
    struct promise_type
    {
        auto get_return_object()
        {
            return p0443_v2::stdcoro::coroutine_handle<promise_type>::from_promise(*this);
        }

        p0443_v2::stdcoro::suspend_never initial_suspend() {
            return {};
        }

        struct final_suspender
        {
            bool await_ready() {
                return false;
            }

            void await_resume() {}

            void await_suspend(p0443_v2::stdcoro::coroutine_handle<promise_type> other) {
                other.destroy();
            }
        };



        final_suspender final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() noexcept
        {
        }

        void return_void() {}
    };

    immediate_task(p0443_v2::stdcoro::coroutine_handle<promise_type> coro): coroutine_(coro) {}

    p0443_v2::stdcoro::coroutine_handle<promise_type> coroutine_;
};
}