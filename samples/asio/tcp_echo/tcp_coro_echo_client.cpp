
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdio>
#include <iostream>

#include <p0443_v2/asio/connect.hpp>
#include <p0443_v2/asio/read_all.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/immediate_task.hpp>

#include <p0443_v2/sink_receiver.hpp>

#include <p0443_v2/sequence.hpp>
#include <p0443_v2/then.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/with.hpp>

namespace net = boost::asio::ip;

p0443_v2::immediate_task client_task(boost::asio::io_context &io, std::string_view host,
                                     std::string_view port, std::string to_send) {
    net::tcp::resolver resolver(io);
    try
    {
        auto eps = co_await p0443_v2::await_sender(p0443_v2::asio::resolve(resolver, host, port));
        net::tcp::socket socket(io);
        auto actual_ep = co_await p0443_v2::await_sender(p0443_v2::asio::connect(socket, eps));
        std::cout << "Connected to " << actual_ep.address().to_string() << ":" << actual_ep.port() << "\n";
        co_await p0443_v2::await_sender(p0443_v2::asio::write_all(socket, boost::asio::buffer(to_send)));
        auto amount_read = co_await p0443_v2::await_sender(p0443_v2::asio::read_all(socket, boost::asio::buffer(to_send)));
        to_send.resize(amount_read);
        std::cout << "Response = '" << to_send << "'\n";
    }
    catch(p0443_v2::await_done_result&) {

    }
    catch(std::exception& ex) {
        std::cout << "Exception: " << ex.what() << "\n";
    }

}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: tcp_echo_client <host> <port> [string to send]\n";
        return 1;
    }

    const std::string string_to_send = [&]() -> const char * {
        if (argc == 3)
            return "hello world";
        return argv[3];
    }();

    boost::asio::io_context io;

    client_task(io, argv[1], argv[2], string_to_send);

    io.run();
}