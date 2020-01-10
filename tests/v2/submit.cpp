#include <doctest/doctest.h>

#include <p0443_v2/execute.hpp>
#include <p0443_v2/schedule.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>

#include "test_receiver.hpp"
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/make_sender.hpp>
#include <p0443_v2/just.hpp>
#include <p0443_v2/via.hpp>

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
    void submit(test_receiver &rcv) {
        rcv.set_value();
    }
};
void submit(free_fn_sender sender, test_receiver &rcv) {
    rcv.submitted = true;
}
TEST_CASE("submit: basic impl") {
    test_receiver test;
    p0443_v2::submit(test_sender(), test);
    REQUIRE(test.submitted);
}

TEST_CASE("submit: via priv_tag") {
    test_receiver test;
    p0443_v2::tag_invoke(test_sender(), p0443_v2::tag::submit, test);
    REQUIRE(test.submitted);
}
TEST_CASE("submit: via ADL") {
    test_receiver test;
    SUBCASE("via tag_invoke") {
        p0443_v2::tag_invoke(free_fn_sender(), p0443_v2::tag::submit, test);
        REQUIRE(test.submitted);
    }
    SUBCASE("via p0443_v2::submit") {
        p0443_v2::submit(free_fn_sender(), test);
        REQUIRE(test.submitted);
    }
}
TEST_CASE("submit: receiver to executor") {
    test_receiver test;
    bool shared = false;
    test.shared_submitted = &shared;
    p0443_v2::submit(test_executor(), test);
    REQUIRE(shared);
}
TEST_CASE("submit: basic impl") {
    test_receiver test;
    p0443_v2::submit(p0443_v2::schedule(test_sender()), test);
    REQUIRE(test.submitted);
}

TEST_CASE("submit: make_receiver") {
    bool set_value_called = false;
    bool set_done_called = false;
    bool set_error_called = false;
    auto recv = p0443_v2::done_channel([&set_done_called]() { set_done_called = true; }) +
                p0443_v2::value_channel([&set_value_called]() { set_value_called = true; }) +
                p0443_v2::error_channel([&set_error_called](auto e) { set_error_called = true; });

    REQUIRE_FALSE(set_value_called);
    REQUIRE_FALSE(set_error_called);
    REQUIRE_FALSE(set_done_called);

    p0443_v2::submit(test_sender(), recv);
    REQUIRE(set_value_called);
    REQUIRE_FALSE(set_error_called);
    REQUIRE_FALSE(set_done_called);

    p0443_v2::submit(p0443_v2::make_sender([](auto &&recv) { p0443_v2::set_done(recv); }), recv);
    REQUIRE(set_done_called);
    REQUIRE(set_value_called);
    REQUIRE_FALSE(set_error_called);

    p0443_v2::submit(p0443_v2::make_sender([](auto &&recv) { p0443_v2::set_error(recv, 1); }),
                     recv);
    REQUIRE(set_done_called);
    REQUIRE(set_value_called);
    REQUIRE(set_error_called);
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