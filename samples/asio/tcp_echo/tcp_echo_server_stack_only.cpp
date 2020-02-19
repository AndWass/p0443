
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/**
 * Note that this example does use the heap in ASIO and via std::cout etc.
 * However no code in this file or in p0443_v2 uses the heap, it is all using
 * stack-based state!
 */
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>

#include <p0443_v2/asio/accept.hpp>
#include <p0443_v2/asio/read_some.hpp>
#include <p0443_v2/asio/write_all.hpp>

#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>

using socket_type = boost::asio::ip::tcp::socket;

template <class Sender, class Receiver>
using operation_type_t =
    decltype(p0443_v2::connect(std::declval<Sender>(), std::declval<Receiver>()));

struct echo_server
{
public:
    echo_server(std::uint16_t port)
        : io_(), acceptor_(io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    }
    void start() {
        std::cout << "Listening on " << acceptor_.local_endpoint().port() << std::endl;
        accept_one();
    }
    void run() {
        io_.run();
    }

private:
    struct client_state
    {
        struct receiver_base
        {
            client_state *owner_;

            void set_done() {
                owner_->stop();
            }
            template <class E>
            void set_error(E &&e) {
                owner_->stop();
            }
        };
        struct read_some_receiver : receiver_base
        {
            void set_value(std::size_t amount_read) {
                std::cout << "Read " << amount_read << " bytes\n";
                this->owner_->start_write(amount_read);
            }
        };

        struct write_all_receiver : receiver_base
        {
            void set_value() {
                this->owner_->start_read();
            }
        };

        socket_type socket_;
        echo_server *server_;
        std::array<std::uint8_t, 64> buffer_;

        using read_operation_t = operation_type_t<decltype(p0443_v2::asio::read_some(
                                                      std::declval<socket_type>(),
                                                      std::declval<boost::asio::mutable_buffer>())),
                                                  read_some_receiver>;
        using write_operation_t = operation_type_t<
            decltype(p0443_v2::asio::write_all(std::declval<socket_type&>(),
                std::declval<boost::asio::const_buffer>())),
            write_all_receiver
        >;

        std::variant<std::monostate, read_operation_t, write_operation_t> operation_states_;

        void start() {
            start_read();
        }

    private:
        void stop() {
            std::cout << "Disconnecting " << socket_.remote_endpoint().address().to_string() << ":"
                      << socket_.remote_endpoint().port() << "\n";
            server_->try_remove_client(this);
        }

        void start_read() {
            operation_states_.template emplace<1>(
                p0443_v2::connect(p0443_v2::asio::read_some(socket_, boost::asio::buffer(buffer_)),
                                  read_some_receiver{this}));
            p0443_v2::start(std::get<1>(operation_states_));
        }

        void start_write(std::size_t amount) {
            operation_states_.template emplace<2>(
                p0443_v2::connect(p0443_v2::asio::write_all(socket_, boost::asio::buffer(buffer_.data(), amount)),
                                  write_all_receiver{this})
            );

            p0443_v2::start(std::get<2>(operation_states_));
        }
    };

    struct accept_receiver
    {
        void set_value(socket_type socket) {
            std::cout << "Connection from " << socket.remote_endpoint().address().to_string() << ":"
                      << socket.remote_endpoint().port() << "\n";
            server_->try_add_client(std::move(socket));
            server_->accept_one();
        }

        void set_done() {
        }

        template <class E>
        void set_error(E &&e) {
        }

        echo_server *server_;
    };

    using connect_op_type = operation_type_t<p0443_v2::asio::accept &&, accept_receiver &&>;

    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::array<std::optional<client_state>, 10> clients_;
    std::optional<connect_op_type> accept_op_;

    void accept_one() {
        accept_op_.emplace(
            p0443_v2::connect(p0443_v2::asio::accept(acceptor_), accept_receiver{this}));
        p0443_v2::start(*accept_op_);
    }

    void try_add_client(socket_type &&socket) {
        for (auto &c : clients_) {
            if (!c.has_value()) {
                c.emplace(client_state{std::move(socket), this});
                c->start();
                break;
            }
        }
    }

    void try_remove_client(client_state *ptr) {
        for (auto &c : clients_) {
            if (c.has_value() && std::addressof(c.value()) == ptr) {
                c.reset();
                break;
            }
        }
    }
};

int main() {
    echo_server server(0);
    server.start();
    server.run();
}