
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "p0443_v2/type_traits.hpp"
#include <boost/system/error_code.hpp>
#include <tuple>
#include <type_traits>

#include <p0443_v2/set_value.hpp>
#include <p0443_v2/set_done.hpp>

namespace p0443_v2::asio
{
template<class Stream, class...Args>
struct handshake
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done =  true;

    Stream* stream_;
    std::tuple<std::remove_reference_t<Args>...> args_;
    handshake(Stream& stream, Args...args): stream_(std::addressof(stream)), args_(std::move(args)...) {}

    template<class Receiver>
    struct operation
    {
        Stream* stream_;
        std::tuple<std::remove_reference_t<Args>...> args_;
        Receiver next_;

        void start() {
            std::apply([this](auto&&...args) {
                auto handler = [next = std::move(next_)](const boost::system::error_code& ec) mutable {
                    if(!ec)  {
                        p0443_v2::set_value(std::move(next));
                    }
                    else {
                        p0443_v2::set_done(std::move(next));
                    }
                };
                stream_->async_handshake(std::forward<decltype(args)>(args)..., std::move(handler));
            }, std::move(args_));
        }
    };

    template<class Receiver>
    auto connect(Receiver&& receiver) {
        return  operation<p0443_v2::remove_cvref_t<Receiver>>{
            stream_, std::move(args_), std::forward<Receiver>(receiver)
        };
    }
};
}