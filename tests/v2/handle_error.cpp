#include <p0443_v2/handle_error.hpp>
#include <p0443_v2/just.hpp>
#include <p0443_v2/make_receiver.hpp>

#include "test_receiver.hpp"

#include <doctest/doctest.h>

TEST_CASE("handle_error: passes through values")
{
    int value = 0;
    auto receiver = p0443_v2::value_channel([&](int val) {
        value = val;
    }) + p0443_v2::done_channel([]() {});
    auto sender = p0443_v2::handle_error(p0443_v2::just(10), [](auto e) { return p0443_v2::just(1); });
    p0443_v2::submit(sender, receiver);
    REQUIRE(value == 10);
}