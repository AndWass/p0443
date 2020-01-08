#include "p0443/task.hpp"
#include "p0443/immediate_executor.hpp"
#include "p0443/execution/receiver_traits.hpp"

#include <doctest/doctest.h>

#include <memory>

template<class...Values>
struct debug_receiver
{
    struct state
    {
        bool value_set = false;
        bool done_set = false;
        bool error_set = false;
    };

    std::shared_ptr<state> shared_state{std::make_shared<state>()};

    void set_value(const Values&...) {
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
    p0443::execution::submit(simple_task, debug_receiver(receiver));
    REQUIRE(task_called);
    REQUIRE(receiver.shared_state->value_set);
    REQUIRE_FALSE(receiver.shared_state->done_set);
    REQUIRE_FALSE(receiver.shared_state->error_set);
}