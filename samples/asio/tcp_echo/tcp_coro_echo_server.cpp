
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>

#include <boost/asio.hpp>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_some.hpp>
#include <p0443_v2/asio/write_all.hpp>

#include <p0443_v2/immediate_task.hpp>

#include <array>

namespace net = boost::asio::ip;

p0443_v2::immediate_task client_task(net::tcp::socket socket) {
    auto ep = socket.remote_endpoint();
    std::cout << "New connection from " << ep.address().to_string() << ":"
              << ep.port() << "\n";
    try
    {
        std::array<std::uint8_t, 128> buffer;
        while(true) {
            auto amount_read = co_await p0443_v2::asio::read_some(socket, boost::asio::buffer(buffer));
            std::cout << "Read " << amount_read << std::endl;
            co_await p0443_v2::asio::write_all(socket, boost::asio::buffer(buffer.data(), amount_read));
        }
    }
    catch(p0443_v2::await_done_result&) {
        std::cout << ep.address().to_string() << ":" << ep.port() << " disconnected\n";
    }
    catch(std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

p0443_v2::immediate_task server_task(boost::asio::io_context &io, std::uint16_t port) {
    namespace net = boost::asio;
    net::ip::tcp::acceptor acceptor(io, net::ip::tcp::endpoint(net::ip::tcp::v4(), port));
    port = acceptor.local_endpoint().port();
    std::cout << "Starting to listen on port " << port << "\n";
    while (true) {
        auto socket = co_await p0443_v2::asio::accept(acceptor);
        client_task(std::move(socket));
    }
}

int main(int argc, char **argv) {
    std::cout << "TCP Echo sample\n";

    std::uint16_t port = 0;
    if (argc >= 2) {
        boost::string_view arg(argv[1]);
        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: samples-asio-tcp-echo [-h] [--help] <port>\n";
            return 0;
        }
        try {
            port = boost::lexical_cast<std::uint16_t>(arg);
        }
        catch (...) {
            std::cout << "Unknown argument: " << arg << "\n";
            return -1;
        }
    }
    namespace net = boost::asio;
    net::io_context io;

    server_task(io, port);

    io.run();
}