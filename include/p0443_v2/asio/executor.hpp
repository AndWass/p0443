#pragma once

#include <p0443_v2/type_traits.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/is_executor.hpp>

#include <type_traits>

namespace p0443_v2::asio
{
namespace detail
{
class executor
{
public:
    explicit executor(boost::asio::executor t): asio_executor_(std::move(t)) {}

    template<class Function>
    void execute(Function &&fn) {
        boost::asio::post(asio_executor_, std::forward<Function>(fn));
    }
private:
    boost::asio::executor asio_executor_;
};
struct executor_fn
{
    template<class T>
    using get_executor_detector = decltype(std::declval<T>().get_executor());

    template<class T,
    std::enable_if_t<
        std::conjunction<
            std::negation<boost::asio::is_executor<T>>,
            p0443_v2::is_detected<get_executor_detector, T>
        >::value
    >* = nullptr>
    auto operator()(T &t) const {
        return executor(t.get_executor());
    }

    template<class T,
        std::enable_if_t<
            boost::asio::is_executor<T>::value
        >* = nullptr
    >
    auto operator()(T &t) const {
        return executor(t);
    }
};
}
constexpr detail::executor_fn executor;
}