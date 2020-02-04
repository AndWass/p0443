
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "submit.hpp"
#include "tag_invoke.hpp"
#include "type_traits.hpp"
#include <type_traits>

namespace p0443_v2
{
namespace detail
{
void schedule();
struct schedule_impl
{
    template<class Sender>
    using member_detector = decltype(std::declval<Sender>().schedule());

    template<class Sender>
    using free_fn_detector = decltype(schedule(std::declval<Sender>()));

    template<class Sender>
    using use_member = p0443_v2::is_detected<member_detector, Sender>;

    template<class Sender>
    using use_free_function = std::conjunction<
        std::negation<use_member<Sender>>,
        p0443_v2::is_detected<free_fn_detector, Sender>
    >;

    template<class Sender>
    using use_argument_copy = std::conjunction<
        std::negation<use_member<Sender>>,
        std::negation<use_free_function<Sender>>
    >;

    template<class Sender, std::enable_if_t<use_member<Sender>::value>* = nullptr>
    auto operator()(Sender &&rx) const noexcept {
        return rx.schedule();
    }

    template<class Sender, std::enable_if_t<use_free_function<Sender>::value>* = nullptr>
    auto operator()(Sender &&rx) const noexcept {
        return schedule(rx);
    }

    template<class Sender, std::enable_if_t<use_argument_copy<Sender>::value>* = nullptr>
    std::decay_t<Sender> operator()(Sender &&rx) const noexcept {
        return std::forward<Sender>(rx);
    }
};
}
constexpr detail::schedule_impl schedule;

namespace tag
{
template<class Sender, std::enable_if_t<std::is_invocable_v<::p0443_v2::detail::schedule_impl, Sender>>* = nullptr>
auto tag_invoke(Sender &&rx, ::p0443_v2::tag::schedule_t)
{
    return ::p0443_v2::schedule(std::forward<Sender>(rx));
}
}

}