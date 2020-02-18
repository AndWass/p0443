#include <p0443_v2/sequence.hpp>
#include <p0443_v2/transform.hpp>

#include <doctest/doctest.h>

#include "test_receiver.hpp"

TEST_CASE("sequence: test sequence") {
    bool s1_called = false;
    bool s2_called = false;
    bool s3_called = false;
    int n = 1;
    int s1_n = 0;
    int s2_n = 0;
    int s3_n = 0;
    bool rx_shared = false;

    auto s1 = p0443_v2::transform(test_sender(), [&] {
        REQUIRE_FALSE(rx_shared);
        REQUIRE_FALSE(s2_called);
        s1_n = n++;
        s1_called = true;
    });

    auto s2 = p0443_v2::transform(test_sender(), [&] {
        REQUIRE(s1_called);
        REQUIRE_FALSE(rx_shared);
        s2_n = n++;
        s2_called = true;
    });

    auto s3 = p0443_v2::transform(test_sender(), [&] {
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

TEST_CASE("sequence: test sequence connect start") {
    bool s1_called = false;
    bool s2_called = false;
    bool s3_called = false;
    int n = 1;
    int s1_n = 0;
    int s2_n = 0;
    int s3_n = 0;
    bool rx_shared = false;

    auto s1 = p0443_v2::transform(test_sender(), [&] {
        REQUIRE_FALSE(rx_shared);
        REQUIRE_FALSE(s2_called);
        s1_n = n++;
        s1_called = true;
    });

    auto s2 = p0443_v2::transform(test_sender(), [&] {
        REQUIRE(s1_called);
        REQUIRE_FALSE(rx_shared);
        s2_n = n++;
        s2_called = true;
    });

    auto s3 = p0443_v2::transform(test_sender(), [&] {
        REQUIRE_FALSE(rx_shared);
        s3_n = n++;
        s3_called = true;
    });

    test_receiver rx;
    rx.shared_submitted = &rx_shared;

    auto seq = p0443_v2::sequence(s1, s2, s3);
    auto op =  p0443_v2::connect(seq, std::move(rx));
    p0443_v2::start(op);

    REQUIRE(s1_called);
    REQUIRE(s2_called);
    REQUIRE(s3_called);
    REQUIRE(rx_shared);
    REQUIRE(s1_n == 1);
    REQUIRE(s2_n == 2);
    REQUIRE(s3_n == 3);
}