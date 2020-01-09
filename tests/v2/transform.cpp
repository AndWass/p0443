#include <p0443/immediate_executor.hpp>
#include <p0443_v2/transform.hpp>

#include <doctest/doctest.h>

#include "test_receiver.hpp"

TEST_CASE("transform: basic transform") {
    test_receiver recv;
    bool transform_called = false;
    bool submitted = false;
    recv.shared_submitted = &submitted;
    p0443_v2::submit(p0443_v2::transform(test_sender(), [&] {
        transform_called = true;
    }), recv);

    REQUIRE(transform_called);
    REQUIRE(submitted);
}
