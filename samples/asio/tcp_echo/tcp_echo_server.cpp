#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <p0443_v2/sink_receiver.hpp>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_some.hpp>
#include <p0443_v2/asio/write_all.hpp>

#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/submit_while.hpp>
#include <p0443_v2/then.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/with.hpp>

#include <array>

namespace net = boost::asio::ip;

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

    // This is what each socket operation will do
    auto connection_operation = [](net::ip::tcp::socket &socket) {
        // Store the remote endpoint so we can use it for printing the done message
        auto ep = socket.remote_endpoint();
        // Use let to ensure both the socket and a read buffer is kept alive for the duration of the
        // socket connection.
        auto single_socket_function = [](auto &socket, auto &buffer) {
            std::cout << "Connection from " << socket.remote_endpoint().port() << "\n";
            // Then takes the values from a sender and uses them as input to the supplied
            // function. The function returns a sender which is then submitted with with the
            // original receiver
            auto write_all = [&socket, &buffer](std::size_t amount_read) mutable {
                return p0443_v2::asio::write_all(socket,
                                                 boost::asio::buffer(buffer.data(), amount_read));
            };
            auto read_write_sender = p0443_v2::then(
                p0443_v2::asio::read_some(socket, net::buffer(buffer)), std::move(write_all));

            // submit_while will continue to resubmit read_write_sender forever,
            // so until done, read some then write all we read and so on
            auto forever = [] { return true; };
            return p0443_v2::submit_while(std::move(read_write_sender), forever);
        };
        auto socket_handler =
            p0443_v2::with(std::move(socket), std::array<char, 128>{}, single_socket_function);
        // So here we have a let sender connected to a function that will return a submit_while
        // sender, this effectively gives us a way to say "while not done do...", which we use to
        // read some data, then write all of that data until we again read some data and so on.
        auto on_connection_done = p0443_v2::done_channel(
            [ep] { std::cout << ep.address().to_string() << ":" << ep.port() << " closed\n"; });
        p0443_v2::submit(std::move(socket_handler), std::move(on_connection_done));
        return true;
    };
    p0443_v2::submit(
        p0443_v2::submit_while(p0443_v2::asio::accept(acceptor), std::move(connection_operation)),
        p0443_v2::sink_receiver{});
    io.run();
}