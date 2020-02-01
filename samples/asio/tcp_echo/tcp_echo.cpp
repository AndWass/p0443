#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/submit_while.hpp>
#include <p0443_v2/let.hpp>
#include <array>

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
    net::ip::tcp::acceptor acceptor(io, net::ip::tcp::endpoint(net::ip::tcp::v4(), port));
    port = acceptor.local_endpoint().port();
    std::cout << "Starting to listen on port " << port << "\n";
    p0443_v2::submit(p0443_v2::submit_while(p0443_v2::asio::accept(acceptor), [](auto &socket) {
        p0443_v2::let(p0443_v2::just(std::move(socket), std::array<char, 128>{}) [](auto &socket, auto &buffer) {
            
            return p0443_v2::submit_while(p0443_v2::sequence(read_some(socket, buffer), write_some(socket, buffer)));
        });
        return true;
    }), p0443_v2::sink_receiver{});
    io.run();
}Ö