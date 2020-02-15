#pragma once

#include <p0443_v2/type_traits.hpp>
#include <type_traits>

namespace p0443_v2
{
namespace detail
{
void connect();

struct connect_cpo
{
    template<class Sender, class Receiver>
    using member_detector = decltype(std::declval<Sender>().connect(std::declval<Receiver>()));

    template<class Sender, class Receiver>
    using free_function_detector = decltype(connect(std::declval<Sender>(), std::declval<Receiver>()));

    template<class Sender, class Receiver>
    using use_member = p0443_v2::is_detected<member_detector, Sender, Receiver>;

    template<class Sender, class Receiver>
    using use_free_function = std::conjunction<
        std::negation<use_member<Sender, Receiver>>,
        p0443_v2::is_detected<free_function_detector, Sender, Receiver>
    >;

    template<class Sender, class Receiver, std::enable_if_t<use_member<Sender, Receiver>::value>* = nullptr>
    auto operator()(Sender&& sender, Receiver&& receiver) const {
        return sender.connect(std::forward<Receiver>(receiver));
    }

    template<class Sender, class Receiver, std::enable_if_t<use_free_function<Sender, Receiver>::value>* = nullptr>
    auto operator()(Sender&& sender, Receiver&& receiver) const {
        return connect(std::forward<Sender>(sender), std::forward<Receiver>(receiver));
    }
};
}
constexpr detail::connect_cpo connect;
}