/**
 * This is a small sample of a simple chat server
 *
 * Each message is sent as lines
 *
 */

#include <cstdio>
#include <string>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_all.hpp>
#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/sink_receiver.hpp>
#include <p0443_v2/submit_while.hpp>

#include <p0443_v2/transform.hpp>
#include <p0443_v2/then.hpp>
#include <p0443_v2/with.hpp>

#include <boost/asio.hpp>

namespace net = boost::asio;

struct chat_room;
struct chat_participant
{
    static constexpr std::size_t header_size = 4;
    net::ip::tcp::socket socket_;
    std::array<char, 512> read_buffer_;
    std::vector<std::string> deliver_queue_;
    bool is_writing_ = false;

    auto read_message() {
        return p0443_v2::then(
            p0443_v2::asio::read_all(socket_, net::buffer(read_buffer_.data(), header_size)),
            [this](std::size_t) {
                read_buffer_[4] = '\0';
                auto message_length = std::atoi(read_buffer_.data());
                if (message_length > read_buffer_.size()) {
                    message_length = 0;
                    socket_.close();
                }
                return p0443_v2::asio::read_all(socket_,
                                                net::buffer(read_buffer_.data(), message_length));
            });
    }

    void deliver(const char* data, std::size_t sz) {
        if(sz == 0) {
            return;
        }
        if(is_writing_) {
            deliver_queue_.emplace_back(data, sz);
        }
        else {
            is_writing_ = true;
            do_deliver(std::string(data, sz));
        }
    }
private:
    void do_deliver(std::string to_deliver) {
        char header[header_size + 1] = "";
        std::sprintf(header, "%4d", static_cast<int>(to_deliver.size()));
        to_deliver.insert(0, header);
        printf("Delivering '%s'\n", to_deliver.c_str());

        auto writer = [this](std::string &str) {
            return p0443_v2::transform(p0443_v2::asio::write_all(socket_, net::buffer(str)), [this]() {
                    if(this->deliver_queue_.empty()) {
                        is_writing_ = false;
                    }
                    else {
                        auto next = std::move(this->deliver_queue_.front());
                        this->deliver_queue_.erase(this->deliver_queue_.begin());
                        this->do_deliver(std::move(next));
                    }
                });
        };

        p0443_v2::submit(p0443_v2::with(std::move(to_deliver), writer), p0443_v2::sink_receiver{});
    }
};

struct chat_room
{
    auto add_participant(chat_participant &participant) {
        participants_.push_back(&participant);
        return p0443_v2::submit_while(participant.read_message(), [this, &participant](std::size_t sz) {
            for(auto* p: participants_) {
                p->deliver(participant.read_buffer_.data(), sz);
            }
            return true;
        });
    }

private:
    std::vector<chat_participant *> participants_;
};

int main(int argc, char **argv) {
    std::uint16_t port = 12345;

    /*if (argc != 2) {
        printf("Usage: telnet_lite_server <port>\n");
        return 1;
    }*/

    port = std::stoi(argv[1]);

    net::io_context io;
    net::ip::tcp::acceptor acceptor(io, net::ip::tcp::endpoint(net::ip::tcp::v4(), port));
    printf("Listening on %d\n", (int)acceptor.local_endpoint().port());

    chat_room room;
    auto infinite_acceptor =
        p0443_v2::submit_while(p0443_v2::asio::accept(acceptor), [&](auto &socket) {
            printf("Connection from %d\n", (int)socket.remote_endpoint().port());
            p0443_v2::submit(p0443_v2::with(chat_participant{std::move(socket)},
                                            [&](chat_participant &participant) {
                                                return room.add_participant(participant);
                                            }),
                             p0443_v2::sink_receiver{});

            return true;
        });

    p0443_v2::submit(infinite_acceptor, p0443_v2::sink_receiver{});
    io.run();
}