#include <p0443_v2/handle_error.hpp>
#include <p0443_v2/just.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/transform.hpp>

#include "test_receiver.hpp"

#include <doctest/doctest.h>
#include <stdexcept>
#include <string>

TEST_CASE("handle_error: passes through a single value") {
    int value = 0;
    auto receiver =
        p0443_v2::value_channel([&](int val) { value = val; }) + p0443_v2::done_channel([]() {});
    auto sender =
        p0443_v2::handle_error(p0443_v2::just(10), [](auto e) { return p0443_v2::just(1); });
    p0443_v2::submit(sender, receiver);
    REQUIRE(value == 10);
}

TEST_CASE("handle_error: submits the returned sender on error") {
    int value = 0;
    std::exception_ptr error;
    auto receiver =
        p0443_v2::value_channel([&](int val) { value = val; });

    auto sender = p0443_v2::handle_error(
        p0443_v2::transform(p0443_v2::just(10),
                            [](auto... vs) -> int { throw std::runtime_error("test"); }),
        [&error](std::exception_ptr e) {
            error = e;
            return p0443_v2::just(1);
        });
    p0443_v2::submit(sender, receiver);
    REQUIRE(value == 1);
    REQUIRE(error);

    try {
        std::rethrow_exception(error);
    }
    catch (std::exception &e) {
        std::string expected = "test";
        REQUIRE(e.what() == expected);
    }
}