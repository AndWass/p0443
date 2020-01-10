#include <p0443/immediate_executor.hpp>
#include <p0443_v2/sequence.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/when_all.hpp>
#include <p0443_v2/when_any.hpp>

#include <doctest/doctest.h>

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

TEST_CASE("sequence: test sequence") {
    auto test = p0443_v2::sequence(test_sender());
    bool s1_called = false;
    bool s2_called = false;
    bool s3_called = false;
    int n = 1;
    int s1_n = 0;
    int s2_n = 0;
    int s3_n = 0;
    bool rx_shared = false;

    auto s1 = p0443_v2::transform_before(test_sender(), [&] {
        REQUIRE_FALSE(rx_shared);
        REQUIRE_FALSE(s2_called);
        s1_n = n++;
        s1_called = true;
    });

    auto s2 = p0443_v2::transform_before(test_sender(), [&] {
        REQUIRE(s1_called);
        REQUIRE_FALSE(rx_shared);
        s2_n = n++;
        s2_called = true;
    });

    auto s3 = p0443_v2::transform_before(test_sender(), [&] {
        REQUIRE_FALSE(rx_shared);
        s3_n = n++;
        s3_called = true;
    });

    test_receiver rx;
    rx.shared_submitted = &rx_shared;

    auto seq = p0443_v2::sequence(s1, s2, s3);
    p0443_v2::submit(seq, rx);

    REQUIRE(s1_called);
    REQUIRE(s2_called);
    REQUIRE(s3_called);
    REQUIRE(rx_shared);
    REQUIRE(s1_n == 1);
    REQUIRE(s2_n == 2);
    REQUIRE(s3_n == 3);
}

TEST_CASE("when_any: basic functionality") {
    bool first_called = false;
    bool second_called = false;
    bool third_called = false;

    counting_receiver recv;

    int counter = 0;

    auto first = p0443_v2::transform_before(test_sender(), [&first_called]() { first_called = true; });
    auto second = p0443_v2::transform_before(test_sender(), [&second_called]() { second_called = true; });
    auto third = p0443_v2::transform_before(test_sender(), [&third_called]() { third_called = true; });

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

    auto first = p0443_v2::transform_before(value_sender<int, int>(1, 2), [&first_called](int, int) { first_called = true; });
    auto second = p0443_v2::transform_before(value_sender<int, int>(3, 4), [&second_called](int, int) { second_called = true; });
    auto third = p0443_v2::transform_before(value_sender<int, int>(5, 6), [&third_called](auto&&...) { third_called = true; });

    auto transformed = p0443_v2::transform(p0443_v2::when_any(first, second, third), [&](int a, int b) {
        REQUIRE(first_called);
        REQUIRE(a == 1);
        REQUIRE(b == 2);
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

    auto first = p0443_v2::transform_before(test_sender(), [&first_called]() { first_called = true; });
    auto second = p0443_v2::transform_before(test_sender(), [&second_called]() { second_called = true; });
    auto third = p0443_v2::transform_before(test_sender(), [&third_called]() { third_called = true; });

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

    auto first = p0443_v2::transform_before(value_sender<int, int>(1, 2), [&first_called](int, int) { first_called = true; });
    auto second = p0443_v2::transform_before(value_sender<int, int>(3, 4), [&second_called](int, int) { second_called = true; });
    auto third = p0443_v2::transform_before(value_sender<int, int>(10, 20), [&third_called](int, int) { third_called = true; });

    auto transformed = p0443_v2::transform(p0443_v2::when_all(first, second, third), [&](int a, int b) {
        REQUIRE(a == 10);
        REQUIRE(b == 20);
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
