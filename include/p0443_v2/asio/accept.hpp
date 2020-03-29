
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <p0443_v2/set_value.hpp>
#include <p0443_v2/set_done.hpp>

namespace p0443_v2::asio
{
namespace net = boost::asio::ip;
struct accept
{
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<net::tcp::socket>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = true;

    accept(net::tcp::acceptor& acceptor): acceptor_(&acceptor) {}

    template<class Receiver>
    struct operation_state
    {
        Receiver receiver_;
        net::tcp::acceptor* acceptor_;

        void start() {
            acceptor_->async_accept([recv = std::move(receiver_)](const auto &ec, net::tcp::socket peer) mutable {
                if(!ec) {
                    p0443_v2::set_value(std::move(recv), std::move(peer));
                }
                else {
                    p0443_v2::set_done(std::move(recv));
                }
            });
        }
    };

    template<class Receiver>
    auto connect(Receiver&& receiver) {
        return operation_state<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(receiver), acceptor_};
    }

    net::tcp::acceptor* acceptor_;
};
}