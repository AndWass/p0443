#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <doctest/doctest.h>

#include <boost/beast.hpp>
#include <p0443_v2/asio/connect.hpp>
#include <p0443_v2/asio/handshake.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/asio/read_some.hpp>
#include <p0443_v2/let.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/sequence.hpp>
#include <p0443_v2/submit.hpp>

TEST_CASE("beast ws connect and handshake") {
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    auto resolve_result = resolver.resolve("demos.kaazing.com", "80");
    boost::beast::websocket::stream<boost::beast::tcp_stream> tcp_stream(ioc);
    auto connect = p0443_v2::asio::connect_socket(tcp_stream.next_layer(), resolve_result);
    auto handshake = p0443_v2::asio::handshake(tcp_stream, "demos.kaazing.com", "/echo");

    bool connect_handshake = false;

    p0443_v2::submit(p0443_v2::sequence(std::move(connect), std::move(handshake)),
                     p0443_v2::value_channel([&]() { connect_handshake = true; }));

    ioc.run();
    REQUIRE(connect_handshake);
}

TEST_CASE("beast ws write and read some") {
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    auto resolve_result = resolver.resolve("demos.kaazing.com", "80");
    boost::beast::websocket::stream<boost::beast::tcp_stream> tcp_stream(ioc);
    auto connect = p0443_v2::asio::connect_socket(tcp_stream.next_layer(), resolve_result);
    auto handshake = p0443_v2::asio::handshake(tcp_stream, "demos.kaazing.com", "/echo");
    auto write =
        p0443_v2::asio::write_all(tcp_stream, boost::asio::buffer("Hello world from p0443_v2"));
    std::string buffer;
    buffer.resize(128);
    auto  read = p0443_v2::asio::read_some(tcp_stream, boost::asio::buffer(buffer));
    bool connect_handshake = false;

    p0443_v2::submit(
        p0443_v2::sequence(std::move(connect), std::move(handshake), std::move(write), std::move(read)),
        p0443_v2::value_channel([&](std::size_t bytes_read) {
            buffer.resize(bytes_read);
            REQUIRE(bytes_read > 0);
            connect_handshake = true;
            }));

    ioc.run();
    REQUIRE(connect_handshake);
}