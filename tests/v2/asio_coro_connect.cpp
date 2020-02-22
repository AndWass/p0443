#include <doctest/doctest.h>

#include <p0443_v2/asio/connect.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/let.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/submit.hpp>

#include <boost/asio.hpp>

#include <iostream>

#include "immediate_task.hpp"

TEST_CASE("asio_coro: connect to local endpoint") {
    using tcp = boost::asio::ip::tcp;
    boost::asio::io_context io;
    tcp::acceptor tcp_acceptor(io, tcp::endpoint(tcp::v4(), 12345));
    tcp_acceptor.listen();
    tcp_acceptor.async_accept([](const auto &ec, tcp::socket socket) {

    });

    bool done = false;

    auto task = [&]() -> immediate_task {
        tcp::socket socket(io);
        tcp::endpoint local(boost::asio::ip::address::from_string("127.0.0.1"), 12345);
        auto ep = co_await p0443_v2::await_sender(p0443_v2::asio::connect_socket(socket, local));
        REQUIRE(ep.port() == 12345);
        done = true;
    };

    task();

    io.run();

    REQUIRE(done);
}

TEST_CASE("asio_coro: resolve and connect to google") {
    using tcp = boost::asio::ip::tcp;

    boost::asio::io_context io;
    bool done = false;
    [&]() -> immediate_task {
        tcp::resolver resolver(io);
        auto result = co_await p0443_v2::await_sender(p0443_v2::asio::resolve(resolver, "www.google.se", "https"));
        tcp::socket socket(io);
        auto ep = co_await p0443_v2::await_sender(p0443_v2::asio::connect_socket(socket, result));
        REQUIRE(ep.port() == 443);
        done = true;
    }();

    io.run();
    REQUIRE(done);
}