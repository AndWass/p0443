
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/type_traits.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/is_executor.hpp>
#include <boost/asio/post.hpp>

#include <type_traits>

namespace p0443_v2::asio
{
namespace detail
{
class executor
{
public:
    explicit executor(boost::asio::executor t) : asio_executor_(std::move(t)) {
    }

    template <class Function>
    void execute(Function &&fn) {
        boost::asio::post(asio_executor_, std::forward<Function>(fn));
    }

private:
    boost::asio::executor asio_executor_;
};
struct executor_fn
{
    template <class T>
    using get_executor_detector = decltype(std::declval<T>().get_executor());

    template <class T>
    auto operator()(T & t) const {
        if constexpr (boost::asio::is_executor<T>::value) {
            return executor(t);
        }
        else if constexpr (p0443_v2::is_detected_v<get_executor_detector, T>) {
            return executor(t.get_executor());
        }
    }
};
} // namespace detail
constexpr detail::executor_fn executor;
} // namespace p0443_v2::asio