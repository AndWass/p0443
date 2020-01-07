#include <p0443/execution/execution.hpp>
#include <p0443/immediate_executor.hpp>

#include <doctest/doctest.h>

namespace test
{
struct test_executor {};
template<class Fn>
void execute(test_executor &ex, Fn&& fn) {
    fn();
}

struct receiver {
    template<class T>
    void submit(T && t) {
        t.set_value();
    }
};
}

TEST_CASE("Member invocation")
{
    bool executed = false;
    SUBCASE("Via execution::execute")
    {
        p0443::immediate_executor executor;
        p0443::execution::execute(executor, [&executed] {
            executed = true;
        });
        REQUIRE(executed);
    }
    SUBCASE("Non-member invocation")
    {
        test::test_executor executor;
        p0443::execution::execute(executor, [&executed] {
            executed = true;
        });
        REQUIRE(executed);
    }
    SUBCASE("using submit")
    {
        test::receiver rx;
        p0443::execution::execute(rx, [&executed] {
            executed = true;
        });
        REQUIRE(executed);
    }
}