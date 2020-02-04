#pragma once

#include <p0443_v2/asio/write_all.hpp>
#include <p0443_v2/asio/read_all.hpp>
#include <p0443_v2/then.hpp>

#include <string>
#include <cstdio>

struct chat_message
{
    std::string message_;

    chat_message() = default;
    explicit chat_message(std::string msg) : message_(std::move(msg)) {
    }

    template <class Socket>
    auto deliver(Socket &socket) {
        char header[5] = "";
        std::snprintf(header, 5, "%4d", static_cast<int>(message_.size()));
        message_.insert(0, header);
        return p0443_v2::asio::write_all(socket, net::buffer(message_));
    }

    template <class Socket>
    auto read(Socket &socket) {
        message_.resize(4);
        auto read_header = p0443_v2::asio::read_all(socket, net::buffer(message_));
        auto read_body = [this, &socket](std::size_t /*header_read_amount = 4*/) {
            auto body_size = std::atoi(message_.c_str());
            printf("Body size = %d bytes\n", (int)body_size);
            fflush(stdout);
            message_.resize(body_size);
            return p0443_v2::asio::read_all(socket, net::buffer(message_));
        };

        return p0443_v2::then(std::move(read_header), std::move(read_body));
    }
};