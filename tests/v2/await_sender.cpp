#include <p0443_v2/await_sender.hpp>
#include <p0443_v2/just.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>

#include <stdexcept>

#include <doctest/doctest.h>

#include "immediate_task.hpp"

TEST_CASE("await_sender: just()")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        co_await p0443_v2::await_sender(p0443_v2::just());
        called = true;
    };
    auto test = task_test();
    REQUIRE(called);
}

TEST_CASE("await_sender: just(int)")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        auto val = co_await p0443_v2::await_sender(p0443_v2::just(10));
        called = true;
        REQUIRE_EQ(val, 10);
    };
    auto test = task_test();
    REQUIRE(called);
}

TEST_CASE("await_sender: just(int, bool)")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        auto [val, boolean] = co_await p0443_v2::await_sender(p0443_v2::just(10, true));
        called = true;
        REQUIRE_EQ(val, 10);
        REQUIRE(boolean);
    };
    auto test = task_test();
    REQUIRE(called);
}

struct done_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = true;

    template<class Receiver>
    struct done_operation
    {
        Receiver receiver_;
        void start() {
            p0443_v2::set_done(std::move(receiver_));
        }
    };
    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return done_operation<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(receiver)};
    }
};

template<class E>
struct error_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<E>;

    static constexpr bool sends_done = true;

    E error_;

    template<class Receiver>
    struct error_operation
    {
        Receiver receiver_;
        E error_;
        void start() {
            p0443_v2::set_error(std::move(receiver_), std::move(error_));
        }
    };
    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return error_operation<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(receiver), std::move(error_)};
    }
};

template<class E>
struct exception_ptr_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = true;

    E error_;

    template<class Receiver>
    struct error_operation
    {
        Receiver receiver_;
        E error_;
        void start() {
            try
            {
                throw error_;
            }
            catch(...)
            {
                p0443_v2::set_error(std::move(receiver_), std::current_exception());
            }
        }
    };
    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return error_operation<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(receiver), std::move(error_)};
    }
};

TEST_CASE("await_sender: done propogated")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        try
        {
            co_await p0443_v2::await_sender(done_sender());
        }
        catch(p0443_v2::await_done_result&) {
            called = true;
        }
    };
    auto test = task_test();
    REQUIRE(called);
}

TEST_CASE("await_sender: error propogated")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        try
        {
            co_await p0443_v2::await_sender(error_sender<std::runtime_error>{std::runtime_error("hello")});
        }
        catch(std::runtime_error&) {
            called = true;
        }
    };
    auto test = task_test();
    REQUIRE(called);
}

TEST_CASE("await_sender: exception_ptr propogated")
{
    bool called = false;
    auto task_test = [&]() -> immediate_task {
        try
        {
            co_await p0443_v2::await_sender(exception_ptr_sender<std::runtime_error>{std::runtime_error("hello")});
        }
        catch(std::runtime_error&) {
            called = true;
        }
    };
    auto test = task_test();
    REQUIRE(called);
}