#include <doctest/doctest.h>

#include <p0443_v2/tag_invoke.hpp>
#include <p0443_v2/submit.hpp>

template<int N>
struct tag_type
{
};

struct test
{
    int N = -1;
    void tag_invoke(tag_type<0>)
    {
        N = 0;
    }
    void tag_invoke(tag_type<1>, bool b) {
        N = 1 + (b ? 1:0);
    }
};
void tag_invoke(test& t, tag_type<10>, int n)
{
    t.N = 10+n;
}

static_assert(p0443_v2::has_tag_invoke_member_function<test, tag_type<0>>::value, "bad has_tag_invoke_member trait");
static_assert(p0443_v2::has_tag_invoke_member_function<test, tag_type<1>, bool>::value, "bad has_tag_invoke_member trait");
static_assert(p0443_v2::has_tag_invoke_free_function<test, tag_type<10>, int>::value, "bad has_tag_invoke_free_function");
static_assert(!p0443_v2::has_tag_invoke_member_function<test, tag_type<100>, bool>::value, "bad has_tag_invoke_member trait");
static_assert(!p0443_v2::has_tag_invoke_free_function<test, tag_type<101>, test>::value, "bad has_tag_invoke_free_function");

TEST_CASE("tag_invoke: member execution")
{
    test t0;
    SUBCASE("test with tag_type<0>")
    {
        p0443_v2::tag_invoke(t0, tag_type<0>());
        REQUIRE(t0.N == 0);
    }
    SUBCASE("test with tag_type<0>, false")
    {
        p0443_v2::tag_invoke(t0, tag_type<1>{}, false);
        REQUIRE(t0.N == 1);
    }
    SUBCASE("test with tag_type<0>, true")
    {
        p0443_v2::tag_invoke(t0, tag_type<1>{}, true);
        REQUIRE(t0.N == 2);
    }
    SUBCASE("test with tag_type<10>, 100")
    {
        p0443_v2::tag_invoke(t0, tag_type<10>{}, 100);
        REQUIRE(t0.N == 110);
    }
}