#include <p0443_v2/connect.hpp>

#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/just.hpp>
#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/make_receiver.hpp>

#include <p0443_v2/start.hpp>

#include <tuple>
#include <variant>
#include <doctest/doctest.h>

static_assert(!p0443_v2::sender_traits<decltype(p0443_v2::just(1))>::sends_done);
static_assert(std::is_same_v<p0443_v2::sender_traits<decltype(p0443_v2::just(1))>::template value_types<std::tuple, std::variant>, std::variant<std::tuple<int>>>);
static_assert(std::is_same_v<p0443_v2::sender_traits<decltype(p0443_v2::just(1))>::template error_types<std::variant>, std::variant<std::exception_ptr>>);

TEST_CASE("just: single value connect")
{
    bool called = false;
    auto op = p0443_v2::connect(p0443_v2::just(), p0443_v2::value_channel([&]() {
        called = true;
    }));
    REQUIRE_FALSE(called);
    p0443_v2::start(op);
    REQUIRE(called);
}

TEST_CASE("just: single value connect")
{
    bool called = false;
    auto op = p0443_v2::connect(p0443_v2::just(10), p0443_v2::value_channel([&](int val) {
        REQUIRE(val == 10);
        called = true;
    }));
    REQUIRE_FALSE(called);
    p0443_v2::start(op);
    REQUIRE(called);
}

TEST_CASE("just: connect multiple values")
{
    bool called = false;
    auto op = p0443_v2::connect(p0443_v2::just(10, 20, 30), p0443_v2::value_channel([&](int x, int y, int z) {
        REQUIRE(x == 10);
        REQUIRE(y == 20);
        REQUIRE(z == 30);
        called = true;
    }));

    REQUIRE_FALSE(called);
    p0443_v2::start(op);
    REQUIRE(called);

}