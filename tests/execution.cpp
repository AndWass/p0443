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

struct sender {
    template<class Rx>
    void submit(Rx&& t) {
        const p0443::immediate_executor imm;
        p0443::execution::submit(imm, std::forward<Rx>(t));
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
        test::sender tx;
        p0443::execution::execute(tx, [&executed] {
            executed = true;
        });
        REQUIRE(executed);
    }
}