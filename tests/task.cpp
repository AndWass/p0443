#include "p0443/task.hpp"
#include "p0443/immediate_executor.hpp"
#include "p0443/asio_executor.hpp"
#include "p0443/execution/receiver_traits.hpp"
#include "p0443/transform.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

#include <doctest/doctest.h>

#include <memory>
#include <iostream>

template<class...Values>
struct debug_receiver
{
    struct state
    {
        bool value_set = false;
        bool done_set = false;
        bool error_set = false;
    };

    mutable std::shared_ptr<state> shared_state{std::make_shared<state>()};

    debug_receiver() = default;
    debug_receiver(const debug_receiver& rhs) = default;
    debug_receiver(debug_receiver&& rhs) noexcept: debug_receiver(rhs) {
    }
    ~debug_receiver() = default;

    debug_receiver &operator=(const debug_receiver&) = default;
    debug_receiver &operator=(debug_receiver&& rhs) noexcept {
        shared_state = rhs.shared_state;
        return *this;
    }


    void set_value(const Values&...) {
        shared_state->value_set = true;
    }

    void set_done() {
        shared_state->done_set = true;
    }
    template <class E>
    void set_error(E) const {
        shared_state->error_set = true;
    }
};

static_assert(p0443::execution::is_receiver<debug_receiver<>>::value, "debug_receiver must satisfy receiver");
static_assert(p0443::execution::is_receiver_of<debug_receiver<int, bool>, int, bool>::value, "debug_receiver<int, bool> must satisfy receiver of int, bool");
static_assert(!p0443::execution::is_receiver_of<debug_receiver<>, int>::value, "debug_receiver<> must not satisfy receiver of int, bool");

struct not_a_receiver {
    void set_done() {}
};
static_assert(!p0443::execution::is_receiver<not_a_receiver>::value, "not_a_receiver should not satisfy is_receiver");

TEST_CASE("simple task") {
    bool task_called = false;
    p0443::immediate_executor exec;
    p0443::task<void, p0443::immediate_executor> simple_task([&task_called]() { task_called = true; }, exec);
    debug_receiver<> receiver;

    REQUIRE_FALSE(task_called);
    p0443::execution::submit(simple_task, receiver);
    REQUIRE(task_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
}

TEST_CASE("task through boost io_context")
{
    bool task_called = false;
    debug_receiver<> receiver;
    boost::asio::io_context io;
    auto simple_task = p0443::make_task([&task_called] {
        task_called = true;
    }, p0443::asio_executor(io));

    REQUIRE_FALSE(task_called);
    p0443::execution::submit(simple_task, receiver);
    REQUIRE_FALSE(task_called);
    REQUIRE_FALSE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);

    io.run();

    REQUIRE(task_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
}

TEST_CASE("task through boost io_context strand")
{
    bool task_called = false;
    debug_receiver<> receiver;
    boost::asio::io_context io;
    boost::asio::io_context::strand strand(io);
    auto simple_task = p0443::make_task([&task_called] {
        task_called = true;
    }, p0443::asio_executor(strand));

    REQUIRE_FALSE(task_called);
    p0443::execution::submit(simple_task, receiver);
    REQUIRE_FALSE(task_called);
    REQUIRE_FALSE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);

    io.run();

    REQUIRE(task_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
}

TEST_CASE("transformed task")
{
    bool task_called = false;
    p0443::immediate_executor exec;
    auto simple_task = p0443::make_task([&task_called]() { task_called = true; }, exec);
    debug_receiver<> receiver;

    bool transform_called = false;
    auto transformed = p0443::transform(simple_task, [&] {
        transform_called = true;
    });

    REQUIRE_FALSE(task_called);
    REQUIRE_FALSE(transform_called);
    p0443::execution::submit(transformed, receiver);
    REQUIRE(task_called);
    REQUIRE(transform_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
}