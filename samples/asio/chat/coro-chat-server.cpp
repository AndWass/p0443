
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cstdio>
#include <deque>
#include <list>
#include <string>
#include <iostream>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_all.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/submit_while.hpp>

#include <p0443_v2/immediate_task.hpp>
#include <p0443_v2/then.hpp>
#include <p0443_v2/transform.hpp>
#include <p0443_v2/with.hpp>

#include <boost/asio.hpp>

#include "chat_message.hpp"

namespace net = boost::asio;

// Helper sender that will always set done
struct always_done
{
    template <class Receiver>
    void submit(Receiver &&recv) {
        p0443_v2::set_done(std::forward<Receiver>(recv));
    }
};

struct chat_participant
{
    net::ip::tcp::socket socket_;
    chat_message read_message_;
    std::deque<chat_message> deliver_queue_;
    bool is_writing_ = false;

    auto read_message() {
        return read_message_.read(socket_);
    }

    void deliver(chat_message msg) {
        deliver_queue_.push_back(std::move(msg));
        if (!is_writing_) {
            is_writing_ = true;
            do_deliver();
        }
    }

private:
    p0443_v2::immediate_task do_deliver() {
        try {
            while (!deliver_queue_.empty()) {
                auto next = deliver_queue_.front();
                deliver_queue_.pop_front();
                co_await p0443_v2::await_sender(next.deliver(socket_));
            }
        }
        catch (...) {
        }
        is_writing_ = false;
    }
};

struct chat_room
{
    void add_participant(chat_participant &participant) {
        participants_.push_back(&participant);
        for(auto m: recent_messages_) {
            participant.deliver(m);
        }
    }

    void remove_participant(chat_participant &participant) {
        auto iter = std::find(participants_.begin(), participants_.end(), &participant);
        if (iter != participants_.end()) {
            participants_.erase(iter);
        }
    }

    void new_message(const chat_participant &from, chat_message message) {
        if (message.message_.empty()) {
            return;
        }

        add_recent_message(message);
        for (auto *p : participants_) {
            if (p != &from) {
                p->deliver(message);
            }
        }
    }

private:
    std::vector<chat_participant *> participants_;

    void add_recent_message(const chat_message &msg) {
        recent_messages_.push_back(msg);
        while (recent_messages_.size() > recent_messages_size_) {
            recent_messages_.pop_front();
        }
    }
    std::deque<chat_message> recent_messages_;
    constexpr static std::size_t recent_messages_size_ = 10;
};

p0443_v2::immediate_task participant_task(net::ip::tcp::socket socket, chat_room &room) {
    auto ep = socket.remote_endpoint();
    std::cout << "Connection from " << ep.address() << ":" << ep.port() << std::endl;

    chat_participant participant{std::move(socket)};
    room.add_participant(participant);
    try {
        while(true) {
            co_await p0443_v2::await_sender(participant.read_message());
            room.new_message(participant, participant.read_message_);
        }
    }
    catch (p0443_v2::await_done_result&) {
        std::cout << "Disconnect from " << ep.address().to_string() << ":" << ep.port() << std::endl;
    }
    catch(std::exception& e) {
        std::cout << "Exception: " << e.what();
    }
    room.remove_participant(participant);
}

p0443_v2::immediate_task room_task(net::io_context &io, std::uint16_t port) {
    try {
        net::ip::tcp::acceptor acceptor(io, net::ip::tcp::endpoint(net::ip::tcp::v4(), port), true);
        port = acceptor.local_endpoint().port();
        printf("Listening on port %u\n", unsigned(port));
        chat_room room;
        while (true) {
            auto socket = co_await p0443_v2::await_sender(p0443_v2::asio::accept(acceptor));
            participant_task(std::move(socket), room);
        }
    }
    catch (p0443_v2::await_done_result &) {
        printf("Shutting down %u\n", unsigned(port));
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: samples-asio-chat-server <port> [<ports>...]\n");
        return 1;
    }

    std::vector<std::uint16_t> ports;
    for (int i = 1; i < argc; i++) {
        try {
            ports.push_back(std::stoi(argv[i]));
        }
        catch (...) {
            printf("Failed to parse port as integer\n");
            return 1;
        }
    }

    net::io_context io;

    for (auto p : ports) {
        room_task(io, p);
    }

    io.run();
}