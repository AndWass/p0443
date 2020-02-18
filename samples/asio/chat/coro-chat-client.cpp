
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

#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/asio/connect.hpp>

#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/submit_while.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/with.hpp>

#include <p0443_v2/immediate_task.hpp>

#include "chat_message.hpp"

namespace net = boost::asio;

struct chat_client
{
    chat_client(net::ip::tcp::socket socket) : socket_(std::move(socket)) {
        read_message_task();
    }
    void send(std::string line) {
        net::dispatch(socket_.get_executor(),
                      [this, line]() mutable { this->write_task(chat_message(std::move(line))); });
    }

private:
    p0443_v2::immediate_task read_message_task() {
        try {
            while(true) {
                chat_message message_to_read;
                co_await p0443_v2::await_sender(message_to_read.read(socket_));
                std::cout << message_to_read.message_ << std::endl;
            }
        }
        catch(...) {}
    }

    p0443_v2::immediate_task write_task(chat_message msg) {
        try
        {
            send_queue_.push_back(std::move(msg));
            if(!is_writing_) {
                is_writing_ = true;
                while(!send_queue_.empty())
                {
                    auto to_send = std::move(send_queue_.front());
                    send_queue_.erase(send_queue_.begin());
                    co_await p0443_v2::await_sender(to_send.deliver(socket_));
                }
                is_writing_ = false;
            }
        }
        catch(...) {}
    }

    net::ip::tcp::socket socket_;
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