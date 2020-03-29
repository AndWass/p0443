#include <doctest/doctest.h>

#include <p0443_v2/execute.hpp>
#include <p0443_v2/schedule.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>

#include "test_receiver.hpp"
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/just.hpp>

#include <iostream>

namespace submit_test
{
struct test_executor
{
    template <class Fn>
    void execute(Fn &&fn) {
        fn();
    }
};

struct free_fn_sender
{
};

TEST_CASE("submit: basic impl") {
    test_receiver test;
    bool b = false;
    test.shared_submitted = &b;
    p0443_v2::submit(test_sender(), test);
    REQUIRE(b);
}

TEST_CASE("submit: basic impl") {
    test_receiver test;
    bool shared_submitted = false;
    test.shared_submitted = &shared_submitted;
    p0443_v2::submit(p0443_v2::schedule(test_sender()), test);
    REQUIRE(test.shared_submitted);
}

TEST_CASE("just: basic usage")
{
    int value = 0;
    auto recv = p0443_v2::value_channel([&value](int val) {
        value = val;
    });

    p0443_v2::submit(p0443_v2::just(10), recv);
    REQUIRE(value == 10);
}

TEST_CASE("just: multiple values usage")
{
    int value = 0;
    auto recv = p0443_v2::value_channel([&value](int val, std::string str) {
        REQUIRE(val == 10);
        REQUIRE(str == "hello");
    });

    p0443_v2::submit(p0443_v2::just(10, "hello"), recv);
}

} // namespace submit_test