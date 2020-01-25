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
template <class Protocol, class Executor = boost::asio::executor>
struct connect
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

    connect(boost::asio::basic_socket<Protocol, Executor> &socket, resolver_results &resolve_results):
        socket_(&socket), endpoints_{&resolve_results} {}

    connect(boost::asio::basic_socket<Protocol, Executor> &socket, endpoint_type &begin):
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
} // namespace p0443_v2::asio