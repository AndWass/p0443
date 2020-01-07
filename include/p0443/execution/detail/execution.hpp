#pragma once

#include "../../util/detect.hpp"

#include <optional>
#include <utility>

#include "execute_impl.hpp"
#include "submit_impl.hpp"
#include "set_value_impl.hpp"
#include "set_done_impl.hpp"
#include "set_error_impl.hpp"

#include "execute_impl.ipp"
#include "submit_impl.ipp"

namespace p0443
{
namespace execution
{
namespace detail
{
/*void set_value();
struct set_value_impl
{
    template <class Rx, class... Args>
    using use_member_t = decltype(std::declval<Rx>().set_value(std::declval<Args>()...));

    template <class Rx, class... Args>
    using use_free_t = decltype(set_value(std::declval<Rx>(), std::declval<Args>()...));

    template <class Rx, class... Args>
    static constexpr bool use_member = util::is_detected_v<use_member_t, Rx, Args...>;

    template <class Rx, class... Args>
    static constexpr bool use_free = !use_member<Rx, Args...> && util::is_detected_v<use_free_t, Rx, Args...>;

    template <class Rx, class... Args>
    std::enable_if_t<use_member<Rx, Args...>> operator()(Rx && rx, Args &&... args) const;

    template <class Rx, class... Args>
    std::enable_if_t<use_free<Rx, Args...>> operator()(Rx && rx, Args &&... args) const;
};*/
} // namespace detail
} // namespace execution
} // namespace p0443