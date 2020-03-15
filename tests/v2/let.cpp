#include <p0443_v2/just.hpp>
#include <p0443_v2/let.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/make_sender.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/sink_receiver.hpp>

#include <iostream>
#include <string>
#include <variant>

#include <doctest/doctest.h>

template<class T>
const char* signature() {
    return __PRETTY_FUNCTION__;
}

TEST_CASE("let: multiple values have extended lifetime") {
    std::string hello = "hello";
    std::string world = "world";
    const std::string *ptr1 = nullptr;
    const std::string *ptr2 = nullptr;
    const std::string *ptr3 = nullptr;
    const std::string *ptr4 = nullptr;
    auto let_sender = p0443_v2::let(p0443_v2::just(hello, world),
                                    [&](std::string &str, const std::string &str2) {
                                        ptr1 = &str;
                                        ptr2 = &str2;
                                        return p0443_v2::transform(p0443_v2::just(10), [](auto v) {
                                            return v*2.0;
                                        });
                                        /*return p0443_v2::make_sender([&](auto &&recv) {
                                            ptr3 = &std::get<0>(*recv.data_);
                                            ptr4 = &std::get<1>(*recv.data_);
                                            p0443_v2::set_value(recv, recv.data_);
                                        });*/
                                    });
    auto just_sender = p0443_v2::just(10);
    auto fn =  [](int) {
        return p0443_v2::just(10.0);
    };

    using shared_type = std::shared_ptr<std::tuple<std::string, std::string>>;
    shared_type stored_ptr;
    /*p0443_v2::submit(let_sender, p0443_v2::value_channel([&](shared_type shared_ptr) {
                         stored_ptr = shared_ptr;
                         REQUIRE(stored_ptr.use_count() >= 2);
                     }));
    REQUIRE(ptr1 == ptr3);
    REQUIRE(ptr2 == ptr4);
    REQUIRE(*ptr1 == hello);
    REQUIRE(*ptr2 == world);*/
}

struct multi_type_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<int, bool>, Tuple<bool, double>, Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    template<class R>
    void submit(R&& r) {}
};

struct multi_receiver_transform
{
    auto operator()(int, bool) {
        return p0443_v2::just(true, true);
    }

    auto operator()(bool, double) {
        return p0443_v2::just();
    }

    auto operator()() {
        return p0443_v2::just(std::tuple<>{});
    }
};