#pragma once

#include <memory>

#include <tuple>
#include <p0443_v2/set_value.hpp>

struct test_receiver
{
    bool submitted = false;
    bool *shared_submitted = nullptr;
    void set_done() {}

    template<class T>
    void set_error(T) {}

    void set_value() {
        submitted = true;
        if(shared_submitted) {
            *shared_submitted = true;
        }
    }
};

struct counting_receiver
{
    struct counters_type
    {
        int set_value = 0;
        int set_done = 0;
        int set_error = 0;
    };

    std::shared_ptr<counters_type> counters{std::make_shared<counters_type>()};

    template<class...Values>
    void set_value(Values&&...values) {
        counters->set_value++;
    }

    void set_done() {
        counters->set_done++;
    }

    template<class E>
    void set_error(E &&e) {
        counters->set_error++;
    }
};

struct test_sender
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;
    
    template<template<class...> class Variant>
    using error_types = Variant<>;

    static constexpr bool sends_done = false;

    template<class Receiver>
    void submit(Receiver &&rcv)
    {
        p0443_v2::set_value(rcv);
    }
};

struct conditional_sender
{
    bool should_set_value = false;
    explicit conditional_sender(bool b): should_set_value(b) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        if(should_set_value) {
            p0443_v2::set_value(recv);
        }
    }
};

template<class...Values>
struct value_sender
{
    std::tuple<std::decay_t<Values>...> val_;

    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<std::decay_t<Values>...>>;
    
    template<template<class...> class Variant>
    using error_types = Variant<>;

    template<class...Vs>
    value_sender(Vs&&... v): val_(std::forward<Vs>(v)...) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        auto all_values = std::tuple_cat(std::forward_as_tuple(std::forward<Receiver>(recv)), val_);
        std::apply(p0443_v2::set_value, std::move(all_values));
    }
};

template<class Value>
struct value_sender<Value>
{
    Value val_;
    value_sender(Value v): val_(v) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        p0443_v2::set_value(recv, val_);
    }
};