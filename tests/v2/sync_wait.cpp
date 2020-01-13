#include <p0443_v2/sync_wait.hpp>
#include <p0443_v2/just.hpp>

#include <doctest/doctest.h>

TEST_CASE("sync_wait: just(5) == 5")
{
    int test = p0443_v2::sync::wait<int>(p0443_v2::just(5));
    REQUIRE(test == 5);
}

TEST_CASE("sync_wait: compile-test just()")
{
    p0443_v2::sync::wait<void>(p0443_v2::just());
}
