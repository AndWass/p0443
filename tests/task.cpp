#include "p0443/task.hpp"
#include "p0443/immediate_executor.hpp"

#include <doctest/doctest.h>

#include <memory>

struct debug_receiver
{
    struct state
    {
        bool value_set = false;
        bool done_set = false;
        bool error_set = false;
    };

    std::shared_ptr<state> shared_state{std::make_shared<state>()};

    template <class... Args>
    void set_value(Args &&...) {
        shared_state->value_set = true;
    }

    void set_done() {
        shared_state->done_set = true;
    }
    template <class E>
    void set_error(E) {
        shared_state->error_set = true;
    }
};

TEST_CASE("simple task") {
    bool task_called = false;
    p0443::immediate_executor exec;
    p0443::task<void, p0443::immediate_executor> simple_task([&task_called]() { task_called = true; }, exec);
    debug_receiver receiver;

    REQUIRE_FALSE(task_called);
    p0443::execution::submit(simple_task, debug_receiver(receiver));
    REQUIRE(task_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
}