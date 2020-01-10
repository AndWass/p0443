#pragma once

#include <tuple>
#include <p0443_v2/make_sender.hpp>
#include <p0443_v2/set_value.hpp>

namespace p0443_v2
{
namespace detail
{
template<class...Values>
struct just_sender
{
    using value_type = std::tuple<std::decay_t<Values>...>;
    value_type val_;

    template<class...Vs>
    just_sender(Vs&&...v): val_(std::forward<Vs>(v)...) {}

    template<class Receiver>
    void submit(Receiver&& recv) & {
        try {
            auto caller = [&recv](auto&&...values) {
                p0443_v2::set_value(recv, values...);
            };
            std::apply(caller, val_);
        }
        catch(...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }

    template<class Receiver>
    void submit(Receiver&& recv) && {
        try {
            auto caller = [&recv](auto&&...values) {
                p0443_v2::set_value(recv, std::move(values)...);
            };
            std::apply(caller, std::move(val_));
        }
        catch(...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }
};

template<class Value>
struct just_sender<Value>
{
    using value_type = std::decay_t<Value>;
    value_type val_;

    template<class Vs>
    just_sender(Vs&& v): val_(std::forward<Vs>(v)) {}

    template<class Receiver>
    void submit(Receiver&& recv) & {
        try {
            p0443_v2::set_value(recv, val_);
        }
        catch(...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }

    template<class Receiver>
    void submit(Receiver&& recv) && {
        try {
            p0443_v2::set_value(recv, std::move(val_));
        }
        catch(...) {
            p0443_v2::set_error(recv, std::current_exception());
        }
    }
};
}

constexpr auto just = [](auto&&...value) {
    return detail::just_sender<decltype(value)...>(std::forward<decltype(value)>(value)...);
};
}