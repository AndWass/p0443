#include <p0443_v2/sender_traits.hpp>
#include <variant>

#include <doctest/doctest.h>
#include <iostream>

struct no_sender_types_test {};
static_assert(!p0443_v2::detail::has_value_types<no_sender_types_test>::value);
static_assert(!p0443_v2::detail::has_error_types<no_sender_types_test>::value);
static_assert(!p0443_v2::detail::has_sends_done<no_sender_types_test>::value);

struct wrong_sends_done_type {
    static constexpr int sends_done = 0;
};
static_assert(!p0443_v2::detail::has_sends_done<wrong_sends_done_type>::value);

struct correct_sends_done_type {
    static constexpr bool sends_done = false;
};
static_assert(p0443_v2::detail::has_sends_done<correct_sends_done_type>::value);

template<class...ErrorTypes>
struct error_sender {
    template<template<class...> class Variant>
    using error_types = Variant<ErrorTypes...>;

    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;
};

static_assert(std::is_same_v<
    std::variant<int, float, double>,
    p0443_v2::append_error_types<std::variant, error_sender<int>, float, double>
>);

static_assert(std::is_same_v<
    std::variant<std::exception_ptr>,
    p0443_v2::append_error_types<std::variant, error_sender<std::exception_ptr>, std::exception_ptr>
>);

template<class...ValueTypes>
struct value_sender {
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<ValueTypes...>>;

    template<template<class...> class Variant>
    using error_types = Variant<>;
};

static_assert(std::is_same_v<
    std::variant<std::tuple<int>, std::tuple<int, bool>, std::tuple<float, double>>,
    p0443_v2::append_value_types<std::tuple, std::variant, value_sender<int>, std::tuple<int, bool>, std::tuple<float, double>, std::tuple<int, bool>>
>);

struct void_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;
};

static_assert(std::is_same_v<std::variant<std::tuple<>>,
    p0443_v2::append_value_types<std::tuple, std::variant, void_sender, std::tuple<>>
>);

static_assert(std::is_same_v<
    std::variant<std::tuple<int, bool>, std::tuple<int, double>>,
    p0443_v2::merge_sender_value_types<std::tuple, std::variant, value_sender<int, bool>, value_sender<int, double>>
>);

template<class T>
const char* fn() {
    return __PRETTY_FUNCTION__;
}

TEST_CASE("sender_traits: test")
{
}