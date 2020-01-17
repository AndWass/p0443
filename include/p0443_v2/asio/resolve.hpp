#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <string>

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/type_traits.hpp>

#include "executor.hpp"

namespace p0443_v2::asio
{
namespace detail
{
struct resolver
{
    using value_type = boost::asio::ip::tcp::resolver::results_type;
    template <class Receiver>
    void submit(Receiver &&recv) {
        if (!resolv_ || host_.empty() || service_.empty()) {
            p0443_v2::set_done(recv);
        }
        else {
            resolv_->async_resolve(host_, service_,
                                   [receiver = p0443_v2::decay_copy(std::forward<Receiver>(recv))](
                                       const auto &ec, auto results) mutable {
                                       if (!ec) {
                                           p0443_v2::set_value(receiver, results);
                                       }
                                       else {
                                           p0443_v2::set_done(receiver);
                                       }
                                   });
        }
    }

    auto schedule() {
        return p0443_v2::asio::executor(*resolv_);
    }

    resolver() = default;

    resolver(boost::asio::ip::tcp::resolver &resolv, std::string_view host,
             std::string_view service)
        : resolv_(& resolv), host_(host), service_(service) {
    }

    boost::asio::ip::tcp::resolver *resolv_;
    std::string host_, service_;
};
} // namespace detail
using resolver = detail::resolver;
} // namespace p0443_v2::asio