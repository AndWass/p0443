#include <p0443/immediate_executor.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/when_all.hpp>
#include <p0443_v2/when_any.hpp>
#include <p0443_v2/just.hpp>

#include <doctest/doctest.h>
#include <iostream>
#include <variant>

#include "test_receiver.hpp"

TEST_CASE("transform: basic transform") {
    test_receiver recv;
    bool transform_called = false;
    bool submitted = false;
    recv.shared_submitted = &submitted;
    p0443_v2::submit(p0443_v2::transform(test_sender(), [&] { transform_called = true; }), recv);

    REQUIRE(transform_called);
    REQUIRE(submitted);
}

namespace test_detail
{
struct sender
{
    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = Variant<Tuple<bool>, Tuple<int>, Tuple<double>>;

    template <template <class...> class Variant>
    using error_types = Variant<>;

    static constexpr bool sends_done = false;

    template <class R>
    void submit(R &&){};
};

struct function
{
    void operator()(bool);
    const char *operator()(int);
    void operator()(double);
    float operator()(float);
};
} // namespace test_detail

TEST_CASE("when_any: basic functionality") {
    bool first_called = false;
    bool second_called = false;
    bool third_called = false;

    counting_receiver recv;

    int counter = 0;

    auto first = p0443_v2::transform(test_sender(), [&first_called]() { first_called = true; });
    auto second = p0443_v2::transform(test_sender(), [&second_called]() { second_called = true; });
    auto third = p0443_v2::transform(test_sender(), [&third_called]() { third_called = true; });

    auto transformed = p0443_v2::transform(p0443_v2::when_any(first, second, third), [&]() {
        REQUIRE(first_called);
        REQUIRE_FALSE(second_called);
        REQUIRE_FALSE(third_called);
        counter++;
    });
    p0443_v2::submit(transformed, recv);

    REQUIRE(recv.counters->set_value == 1);
    REQUIRE(counter == 1);
    REQUIRE(first_called);
    REQUIRE(second_called);
    REQUIRE(third_called);
}

TEST_CASE("when_any: with values functionality") {
    bool first_called = false;
    bool second_called = false;
    bool third_called = false;

    counting_receiver recv;

    int counter = 0;

    auto first = p0443_v2::transform(value_sender<int, int>(1, 2), [&first_called](int, int) {
        first_called = true;
        return 1;
    });
    auto second = p0443_v2::transform(value_sender<int, int>(3, 4), [&second_called](int, int) {
        second_called = true;
        return 2;
    });
    auto third = p0443_v2::transform(value_sender<int, int>(5, 6), [&third_called](auto &&...) {
        third_called = true;
        return 3;
    });

    auto transformed = p0443_v2::transform(p0443_v2::when_any(first, second, third), [&](int n) {
        REQUIRE(n == 1);
        REQUIRE(first_called);
        REQUIRE_FALSE(second_called);
        REQUIRE_FALSE(third_called);
        counter++;
    });
    p0443_v2::submit(transformed, recv);

    REQUIRE(recv.counters->set_value == 1);
    REQUIRE(counter == 1);
    REQUIRE(first_called);
    REQUIRE(second_called);
    REQUIRE(third_called);
}

TEST_CASE("when_all: basic functionality") {
    bool first_called = false;
    bool second_called = false;
    bool third_called = false;

    counting_receiver recv;

    int counter = 0;

    auto first = p0443_v2::transform(test_sender(), [&first_called]() { first_called = true; });
    auto second = p0443_v2::transform(test_sender(), [&second_called]() { second_called = true; });
    auto third = p0443_v2::transform(test_sender(), [&third_called]() { third_called = true; });

    auto transformed = p0443_v2::transform(p0443_v2::when_all(first, second, third), [&]() {
        REQUIRE(first_called);
        REQUIRE(second_called);
        REQUIRE(third_called);
        counter++;
    });
    p0443_v2::submit(transformed, recv);

    REQUIRE(recv.counters->set_value == 1);
    REQUIRE(counter == 1);
    REQUIRE(first_called);
    REQUIRE(second_called);
    REQUIRE(third_called);
}

TEST_CASE("when_all: with values") {
    bool first_called = false;
    bool second_called = false;
    bool third_called = false;

    counting_receiver recv;

    int counter = 0;

    auto first = p0443_v2::transform(value_sender<int, int>(1, 2), [&first_called](int, int) {
        first_called = true;
        return 1;
    });
    auto second = p0443_v2::transform(value_sender<int, int>(3, 4), [&second_called](int, int) {
        second_called = true;
        return 2;
    });
    auto third = p0443_v2::transform(value_sender<int, int>(10, 20), [&third_called](int, int) {
        third_called = true;
        return 3;
    });

    auto transformed = p0443_v2::transform(p0443_v2::when_all(first, second, third), [&](int a) {
        REQUIRE(a == 3);
        REQUIRE(first_called);
        REQUIRE(second_called);
        REQUIRE(third_called);
        counter++;
    });
    p0443_v2::submit(transformed, recv);

    REQUIRE(recv.counters->set_value == 1);
    REQUIRE(counter == 1);
    REQUIRE(first_called);
    REQUIRE(second_called);
    REQUIRE(third_called);
}
