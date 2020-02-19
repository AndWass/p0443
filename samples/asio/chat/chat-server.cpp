
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cstdio>
#include <list>
#include <string>
#include <deque>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_all.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/submit_while.hpp>

#include <p0443_v2/handle_done.hpp>
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
    std::vector<chat_message> deliver_queue_;
    bool is_writing_ = false;

    auto read_message() {
        return read_message_.read(socket_);
    }

    void deliver(chat_message msg) {
        if (msg.message_.empty()) {
            return;
        }
        if (is_writing_) {
            deliver_queue_.emplace_back(std::move(msg));
        }
        else {
            is_writing_ = true;
            do_deliver(std::move(msg));
        }
    }

private:
    void do_deliver(chat_message to_deliver) {
        // the msg reference will live until the sender returned from writer has been
        // completed. This is ensured by using with below.
        auto writer = [this](chat_message &msg) {
            return p0443_v2::transform(msg.deliver(this->socket_), [this]() {
                if (this->deliver_queue_.empty()) {
                    is_writing_ = false;
                }
                else {
                    auto next = std::move(this->deliver_queue_.front());
                    this->deliver_queue_.erase(this->deliver_queue_.begin());
                    this->do_deliver(std::move(next));
                }
            });
        };
        p0443_v2::submit(p0443_v2::with(writer, std::move(to_deliver)), p0443_v2::sink_receiver{});
    }
};

struct chat_room
{
    auto participant_handler_for(chat_participant &participant) {
        participants_.push_back(&participant);

        for(const auto &msg: recent_messages_) {
            participant.deliver(msg);
        }

        auto participant_remover = [this, &participant]() {
            auto address = participant.socket_.remote_endpoint().address().to_string();
            auto port = participant.socket_.remote_endpoint().port();

            printf("Disconnect from %s:%d\n", address.c_str(), static_cast<int>(port));
            fflush(stdout);

            participants_.erase(
                std::remove(participants_.begin(), participants_.end(), &participant),
                participants_.end());
            return always_done{};
        };

        auto message_reader =
            p0443_v2::submit_while([&participant] { return participant.read_message(); },
                                   [this, &participant](std::size_t /*body_size*/) {
                                       for (auto *p : participants_) {
                                           if (p != &participant) {
                                               p->deliver(participant.read_message_);
                                           }
                                       }
                                       add_recent_message(participant.read_message_);
                                       return true;
                                   });

        return p0443_v2::handle_done(std::move(message_reader), std::move(participant_remover));
    }

private:
    std::vector<chat_participant *> participants_;

    void add_recent_message(const chat_message& msg) {
        recent_messages_.push_back(msg);
        while(recent_messages_.size() > recent_messages_size_)
        {
            recent_messages_.pop_front();
        }
    }
    std::deque<chat_message> recent_messages_;
    constexpr static std::size_t recent_messages_size_ = 10;
};

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

        auto room_handler = [](auto &acceptor, auto &current_room) {
            printf("Listening on %d\n", (int)acceptor.local_endpoint().port());
            fflush(stdout);

            auto socket_handler = [&current_room](net::ip::tcp::socket &socket) {
                auto address = socket.remote_endpoint().address().to_string();

                printf("Connection from %s:%d\n", address.c_str(),
                       (int)socket.remote_endpoint().port());
                fflush(stdout);

                auto participant_handler = [&socket, &current_room](chat_participant &participant) {
                    return current_room.participant_handler_for(participant);
                };
                p0443_v2::submit(
                    p0443_v2::with(participant_handler, chat_participant{std::move(socket)}),
                    p0443_v2::sink_receiver{});

                return true;
            };

            return p0443_v2::submit_while([&acceptor] { return p0443_v2::asio::accept(acceptor); },
                                          std::move(socket_handler));
        };

        auto acceptor = p0443_v2::with(
            room_handler, net::ip::tcp::acceptor(io, net::ip::tcp::endpoint(net::ip::tcp::v4(), p)),
            chat_room());

        p0443_v2::submit(std::move(acceptor), p0443_v2::sink_receiver{});
    }

    io.run();
}