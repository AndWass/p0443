#pragma once

#include "../../util/detect.hpp"

namespace p0443::execution::detail
{
void execute();
struct execute_impl
{
    template <class Ex, class Fn, class NoRef = std::remove_reference_t<Ex>>
    using member_execute_t = decltype(std::declval<NoRef>().execute(std::declval<Fn>()));

    template <class Ex, class Fn>
    using free_execute_t = decltype(execute(std::declval<Ex>(), std::declval<Fn>()));

    template <class Ex, class Fn>
    static constexpr bool use_member = util::is_detected_v<member_execute_t, Ex, Fn>;

    template <class Ex, class Fn>
    static constexpr bool use_free = !use_member<Ex, Fn> && util::is_detected_v<free_execute_t, Ex, Fn>;

    template <class Ex, class Fn>
    static constexpr bool use_submit = !use_free<Ex, Fn> && !use_member<Ex, Fn>;

    template <typename Ex, typename Fn>
    std::enable_if_t<use_member<Ex, Fn>> operator()(Ex && ex, Fn && fn) const;

    template <typename Ex, typename Fn>
    std::enable_if_t<use_free<Ex, Fn>> operator()(Ex && ex, Fn && fn) const;

    template <typename Ex, typename Fn>
    std::enable_if_t<use_submit<Ex, Fn>> operator()(Ex && ex, Fn && fn) const;
};
}