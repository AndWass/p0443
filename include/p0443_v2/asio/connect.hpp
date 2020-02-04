
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <vector>
#include <variant>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver_results.hpp>

#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_value.hpp>

namespace p0443_v2::asio
{
namespace detail
{
template <class Protocol, class Executor>
struct connect_sender
{
    using socket_type = boost::asio::basic_socket<Protocol, Executor>;
    using endpoint_type = boost::asio::ip::basic_endpoint<Protocol>;
    using resolver_results = boost::asio::ip::basic_resolver_results<Protocol>;
    socket_type *socket_;
    struct endpoint_range
    {
        const endpoint_type *begin_ = nullptr;
        const endpoint_type *end_ = nullptr;
    };
    std::variant<endpoint_range, const resolver_results*> endpoints_;

    using value_types = std::variant<endpoint_type>;
    using error_types = std::variant<std::exception_ptr>;

    connect_sender(boost::asio::basic_socket<Protocol, Executor> &socket, resolver_results &resolve_results):
        socket_(&socket), endpoints_{&resolve_results} {}

    connect_sender(boost::asio::basic_socket<Protocol, Executor> &socket, endpoint_type &begin):
        socket_(&socket), endpoints_(endpoint_range{&begin, (&begin)+1}) {}

    template <class Receiver>
    void submit(Receiver &&receiver) {
        if(endpoints_.index() == 0) {
            submit_endpoint_range(std::forward<Receiver>(receiver));
        }
        else {
            submit_resolver_results(std::forward<Receiver>(receiver));
        }
    }
private:
    template<class Receiver>
    void submit_endpoint_range(Receiver &&receiver)
    {
        endpoint_range ep = std::get<0>(endpoints_);
        if(ep.begin_) {
            boost::asio::async_connect(*socket_, ep.begin_, ep.end_,
                                        [end = ep.end_, receiver](const auto &ec, const auto ep) mutable {
                                            if (ep == end) {
                                                p0443_v2::set_done(receiver);
                                            }
                                            else {
                                                p0443_v2::set_value(receiver, *ep);
                                            }
                                        });
        }
        else {
            p0443_v2::set_done(std::forward<Receiver>(receiver));
        }
    }

    template<class Receiver>
    void submit_resolver_results(Receiver &&receiver)
    {
        const resolver_results* ep = std::get<1>(endpoints_);
        if(ep) {
            boost::asio::async_connect(*socket_, *ep,
                                        [receiver](const auto &ec, const auto& ep) mutable {
                                            if (ep == endpoint_type()) {
                                                p0443_v2::set_done(receiver);
                                            }
                                            else {
                                                p0443_v2::set_value(receiver, ep);
                                            }
                                        });
        }
        else {
            p0443_v2::set_done(std::forward<Receiver>(receiver));
        }
    }
};

struct connect_cpo
{
    template<class Protocol, class Executor, class Endpoint>
    auto operator()(boost::asio::basic_socket<Protocol, Executor> &socket, Endpoint& ep) const {
        return connect_sender<Protocol, Executor>(socket, ep);
    }
};
}

constexpr detail::connect_cpo connect;
} // namespace p0443_v2::asio