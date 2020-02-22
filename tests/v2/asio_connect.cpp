#include <doctest/doctest.h>

#include <p0443_v2/asio/connect.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/let.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/submit.hpp>

#include <boost/asio.hpp>

#include <iostream>

TEST_CASE("connect to local endpoint") {
    using tcp = boost::asio::ip::tcp;
    boost::asio::io_context io;
    tcp::acceptor tcp_acceptor(io, tcp::endpoint(tcp::v4(), 12345));
    tcp_acceptor.listen();
    tcp_acceptor.async_accept([](const auto &ec, tcp::socket socket) {

    });

    tcp::socket socket(io);
    tcp::endpoint local(boost::asio::ip::address::from_string("127.0.0.1"), 12345);
    auto connect = p0443_v2::asio::connect_socket(socket, local);

    p0443_v2::submit(connect, p0443_v2::value_channel([&socket](tcp::socket::endpoint_type ep) {
                         REQUIRE(ep.port() == 12345);
                     }));

    io.run();
}

TEST_CASE("connect to local endpoint") {
    using tcp = boost::asio::ip::tcp;
    boost::asio::io_context io;
    tcp::resolver resolver(io);
    tcp::socket socket(io);
    auto resolve_connect =
        p0443_v2::let(p0443_v2::asio::resolve(resolver, "www.google.se", "https"),
                      [&](tcp::resolver::results_type &results) {
                          // Results will continue to live for the duration of
                          // the connect sender
                          return p0443_v2::asio::connect_socket(socket, results);
                      });

    p0443_v2::submit(resolve_connect, p0443_v2::value_channel([&](tcp::socket::endpoint_type ep) {
                         REQUIRE(ep.port() == 443);
                     }));

    io.run();
}