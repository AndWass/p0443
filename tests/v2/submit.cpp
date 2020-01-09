#include <doctest/doctest.h>

#include <p0443_v2/execute.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/schedule.hpp>

#include "test_receiver.hpp"

namespace submit_test
{
struct test_sender
{
    void submit(test_receiver &rcv)
    {
        rcv.set_value();
    }
};

struct test_executor
{
    template<class Fn>
    void execute(Fn &&fn) {
        fn();
    }
};

struct free_fn_sender
{
    void submit(test_receiver &rcv) {
        rcv.set_value();
    }
};
void submit(free_fn_sender sender, test_receiver &rcv) {
    rcv.submitted = true;
}
TEST_CASE("submit: basic impl")
{
    test_receiver test;
    p0443_v2::submit(test_sender(), test);
    REQUIRE(test.submitted);
}

TEST_CASE("submit: via priv_tag")
{
    test_receiver test;
    p0443_v2::tag_invoke(test_sender(), p0443_v2::tag::submit, test);
    REQUIRE(test.submitted);
}
TEST_CASE("submit: via ADL")
{
    test_receiver test;
    SUBCASE("via tag_invoke")
    {
        p0443_v2::tag_invoke(free_fn_sender(), p0443_v2::tag::submit, test);
        REQUIRE(test.submitted);
    }
    SUBCASE("via p0443_v2::submit")
    {
        p0443_v2::submit(free_fn_sender(), test);
        REQUIRE(test.submitted);
    }
}
TEST_CASE("submit: receiver to executor")
{
    test_receiver test;
    bool shared = false;
    test.shared_submitted = &shared;
    p0443_v2::submit(test_executor(), test);
    REQUIRE(shared);
}
TEST_CASE("submit: basic impl")
{
    test_receiver test;
    p0443_v2::submit(p0443_v2::schedule(test_sender()), test);
    REQUIRE(test.submitted);
}
}