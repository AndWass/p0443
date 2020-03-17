
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <boost/asio.hpp>

#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/submit_while.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/with.hpp>
#include <p0443_v2/ignore_values.hpp>

#include "chat_message.hpp"

namespace net = boost::asio;

struct chat_client
{
    chat_client(net::ip::tcp::socket socket) : socket_(std::move(socket)) {
        p0443_v2::submit(p0443_v2::submit_while([&]() { return message_to_read.read(socket_); },
                                                [&](std::size_t) {
                                                    printf("%s\n",
                                                           message_to_read.message_.c_str());
                                                    fflush(stdout);

                                                    return true;
                                                }),
                         p0443_v2::sink_receiver{});
    }
    void send(std::string line) {
        net::dispatch(socket_.get_executor(),
                      [this, line]() mutable { this->do_send(chat_message(std::move(line))); });
    }

private:
    void do_send(chat_message message) {
        if (!is_writing_) {
            is_writing_ = true;

            auto write_sender = p0443_v2::with(
                [this](chat_message& msg) { return msg.deliver(socket_); }, std::move(message));

            auto send_next = [this]() -> void {
                is_writing_ = false;
                if (!send_queue_.empty()) {
                    auto front = std::move(send_queue_.front());
                    send_queue_.erase(send_queue_.begin());
                    do_send(std::move(front));
                }
            };
            auto continuation = p0443_v2::transform(p0443_v2::ignore_values(std::move(write_sender)), std::move(send_next));

            p0443_v2::submit(std::move(continuation), p0443_v2::sink_receiver{});
        }
    }
    net::ip::tcp::socket socket_;
    chat_message message_to_read;
    std::vector<chat_message> send_queue_;
    bool is_writing_ = false;
};

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: samples-asio-chat-client <host> <port>\n");
        return 1;
    }

    std::string_view host = argv[1];
    std::string_view port = argv[2];

    net::io_context io;
    net::ip::tcp::resolver resolver(io);
    auto resolve_results = resolver.resolve(net::ip::tcp::v4(), host, port);
    net::ip::tcp::socket socket(io);
    
    for (auto &r : resolve_results) {
        try {
            socket.connect(r);
            break;
        }
        catch (...) {
        }
    }
    if (!socket.is_open()) {
        printf("Failed to open socket");
        return 1;
    }

    chat_client client(std::move(socket));

    std::thread io_thread([&]() { io.run(); });

    std::string line;
    while (std::getline(std::cin, line)) {
        client.send(line);
    }
    io.stop();
    io_thread.join();
}