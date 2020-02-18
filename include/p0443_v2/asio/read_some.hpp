
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

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

    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<std::size_t>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done =  true;

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