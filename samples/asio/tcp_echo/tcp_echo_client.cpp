#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>

#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/asio/connect.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/asio/read_some.hpp>

#include <p0443_v2/then.hpp>
#include <p0443_v2/transform.hpp>

namespace net = boost::asio::ip;

int main(int argc, char **argv)
{
    if(argc != 3) {
        std::cout << "Usage: tcp_echo_client <host> <port>\n";
        return 1;
    }

    boost::asio::io_context io;
    net::tcp::resolver resolver(io);
    net::tcp::socket socket(io);
    std::array<char, 128> read_buffer;

    auto resolve_sender = p0443_v2::asio::resolve(resolver, argv[1], argv[2]);
    auto connector = p0443_v2::then(std::move(resolve_sender), [&](auto& results) {
        return p0443_v2::then(p0443_v2::asio::connect(socket, results), [&](auto& ep) {
            std::cout << "Connected to " << ep.address().to_string() << ":" << ep.port() << "\n";
            return p0443_v2::then(p0443_v2::asio::write_all(socket, boost::asio::buffer("hello world!")), [&]() {
                return p0443_v2::transform(p0443_v2::asio::read_some(socket, boost::asio::buffer(read_buffer)), [&](std::size_t read_amount) {
                    boost::string_view sv(read_buffer.data(), read_amount);
                    std::cout << sv << "\n";
                    return 0;
                });
            });
        });
    });

    p0443_v2::submit(std::move(connector), p0443_v2::sink_receiver{});

    io.run();
}