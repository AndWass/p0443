#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>

namespace p0443_v2::asio
{
namespace detail
{
template<class Stream>
struct write_all_sender
{
    Stream *stream_;
    boost::asio::const_buffer buffer_;

    template<class Buffer>
    write_all_sender(Stream& stream, Buffer&& buffer): stream_(&stream), buffer_(buffer) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        boost::asio::async_write(*stream_, buffer_, [recv = std::forward<Receiver>(recv)](const auto &ec, std::size_t bytes_written) mutable {
            if(!ec) {
                p0443_v2::set_value(std::move(recv));
            }
            else {
                p0443_v2::set_done(std::move(recv));
            }
        });
    }
};

struct write_all_cpo
{
    template<class Stream, class Buffer>
    auto operator()(Stream &stream, Buffer &&buffer) const {
        return write_all_sender<Stream>(stream, std::forward<Buffer>(buffer));
    }
};
}
constexpr detail::write_all_cpo write_all;
}