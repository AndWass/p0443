#pragma once

#include <boost/asio/buffer.hpp>

namespace p0443_v2::asio
{
namespace detail
{
template<class Stream>
struct read_some_sender
{
    Stream *stream_;
    boost::asio::mutable_buffer buffer_;

    template<class Buffer>
    read_some_sender(Stream& stream, Buffer&& buffer): stream_(&stream), buffer_(buffer) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        stream_->async_read_some(buffer_, [recv = std::forward<Receiver>(recv)](const auto &ec, std::size_t bytes_read) mutable {
            if(!ec) {
                p0443_v2::set_value(std::move(recv), bytes_read);
            }
            else {
                p0443_v2::set_done(std::move(recv));
            }
        });
    }
};

struct read_some_cpo
{
    template<class Stream, class Buffer>
    auto operator()(Stream &stream, Buffer &&buffer) const {
        return read_some_sender<Stream>(stream, std::forward<Buffer>(buffer));
    }
};
}
constexpr detail::read_some_cpo read_some;
}