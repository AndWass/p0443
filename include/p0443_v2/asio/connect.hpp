
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <vector>
#include <variant>

#include <boost/type_traits/is_detected.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver_results.hpp>

#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_value.hpp>

namespace p0443_v2::asio
{
namespace detail
{
void async_connect();

struct connect_socket_cpo
{
    template<class Socket, class...Args>
    using member_detector = decltype(std::declval<Socket>().async_connect(std::declval<Args>()...));

    template<class Socket, class...Args>
    using use_member = boost::is_detected<member_detector, Socket, Args...>;

    template<class Socket, class...Args>
    auto operator()(Socket& sock, Args&&...args) const
    {
        if constexpr(use_member<Socket, Args...>::value)
        {
            return sock.async_connect(std::forward<Args>(args)...);
        }
        else {
            return async_connect(sock, std::forward<Args>(args)...);
        }
    }
};
static constexpr connect_socket_cpo connect_socket_impl;
}
/**
 * @brief Connects a socket to an endpoint.
 *
 * The socket must live for the duration of this operation.
 *
 * Sender value: nete::basic_endpoint<Stream::protocol_type>
 * Sender error: std::exception_ptr
 * Sender sets done: Yes
 */
template <class Stream>
struct connect_socket
{
    using socket_type = Stream;
    using protocol_type = typename Stream::protocol_type;
    using endpoint_type = boost::asio::ip::basic_endpoint<protocol_type>;
    using resolver_results = boost::asio::ip::basic_resolver_results<protocol_type>;
    socket_type *socket_;
    struct endpoint_range
    {
        const endpoint_type *begin_ = nullptr;
        const endpoint_type *end_ = nullptr;
    };
    std::variant<endpoint_range, const resolver_results*> endpoints_;

    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<endpoint_type>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = true;

    connect_socket(Stream &socket, resolver_results &resolve_results):
        socket_(&socket), endpoints_{&resolve_results} {}

    connect_socket(Stream &socket, endpoint_type &begin):
        socket_(&socket), endpoints_(endpoint_range{&begin, (&begin)+1}) {}

    template<class Receiver>
    struct operation_state
    {
        Receiver receiver_;
        std::variant<endpoint_range, const resolver_results*> endpoints_;
        socket_type *socket_;

        void start() {
            if(endpoints_.index() == 0) {
                submit_endpoint_range();
            }
            else {
                submit_resolver_results();
            }
        }

        void submit_endpoint_range()
        {
            endpoint_range ep = std::get<0>(endpoints_);
            if(ep.begin_) {
                detail::connect_socket_impl(*socket_, ep.begin_, ep.end_,
                                            [end = ep.end_, receiver = std::move(receiver_)](const auto &ec, const auto ep) mutable {
                                                if (ep == end) {
                                                    p0443_v2::set_done(std::move(receiver));
                                                }
                                                else {
                                                    p0443_v2::set_value(std::move(receiver), *ep);
                                                }
                                            });
            }
            else {
                p0443_v2::set_done(std::move(receiver_));
            }
        }
        void submit_resolver_results()
        {
            const resolver_results* ep = std::get<1>(endpoints_);
            if(ep) {
                detail::connect_socket_impl(*socket_, *ep,
                                            [receiver = std::move(receiver_)](const auto &ec, const auto& ep) mutable {
                                                if (ep == endpoint_type()) {
                                                    p0443_v2::set_done(std::move(receiver));
                                                }
                                                else {
                                                    p0443_v2::set_value(std::move(receiver), ep);
                                                }
                                            });
            }
            else {
                p0443_v2::set_done(std::move(receiver_));
            }
        }
    };

    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return operation_state<p0443_v2::remove_cvref_t<Receiver>>{std::forward<Receiver>(receiver), endpoints_, socket_};
    }
private:
    template<class Receiver>
    void submit_endpoint_range(Receiver &&receiver)
    {
        endpoint_range ep = std::get<0>(endpoints_);
        if(ep.begin_) {
            detail::connect_socket_impl(*socket_, ep.begin_, ep.end_,
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
            detail::connect_socket_impl(*socket_, *ep,
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
} // namespace p0443_v2::asio