
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
#include <p0443_v2/asio/read_some.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/sink_receiver.hpp>

#include <p0443_v2/with.hpp>
#include <p0443_v2/then.hpp>
#include <p0443_v2/sequence.hpp>
#include <p0443_v2/transform.hpp>

namespace net = boost::asio::ip;

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
    struct read_state
    {
        net::tcp::resolver resolver;
        net::tcp::socket socket;
        std::string read_buffer;

        read_state(boost::asio::io_context &io) : resolver(io), socket(io) {
        }
    };

    auto resolve_connect = [](auto &resolver, auto &socket, auto host, auto service) {
        return p0443_v2::then(p0443_v2::asio::resolve(resolver, host, service), [&](auto &eps) {
            return p0443_v2::asio::connect(socket, eps);
        });
    };

    auto read_print = [&](read_state& state) {
        return p0443_v2::transform(p0443_v2::asio::read_some(state.socket, boost::asio::buffer(state.read_buffer)), [&](std::size_t read_amount) {
            std::printf("Data read = '%.*s'\n", static_cast<int>(read_amount), state.read_buffer.data());
        });
    };

    auto write_read = [&](read_state& state) {
        return p0443_v2::sequence(p0443_v2::asio::write_all(state.socket, boost::asio::buffer(string_to_send)), read_print(state));
    };

    // With is a simplification where
    // with(fn, value1, value2) = let(just(value1, value2), fn)
    auto connector = p0443_v2::with([&](read_state& state) {
        state.read_buffer.resize(string_to_send.size());
        return p0443_v2::sequence(resolve_connect(state.resolver, state.socket, argv[1], argv[2]), write_read(state));
    }, read_state(io));

    p0443_v2::submit(std::move(connector), p0443_v2::done_channel([] {
        std::printf("Done\n");
    }));

    io.run();
}