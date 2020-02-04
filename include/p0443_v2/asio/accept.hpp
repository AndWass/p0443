
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
    accept(net::tcp::acceptor& acceptor): acceptor_(&acceptor) {}

    template<class Receiver>
    void submit(Receiver &&receiver) {
        acceptor_->async_accept([recv = std::move(receiver)](const auto &ec, net::tcp::socket peer) mutable {
            if(!ec) {
                p0443_v2::set_value(std::move(recv), std::move(peer));
            }
            else {
                p0443_v2::set_done(std::move(recv));
            }
        });
    }

    net::tcp::acceptor* acceptor_;
};
}