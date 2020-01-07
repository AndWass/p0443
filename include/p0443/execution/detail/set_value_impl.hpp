#pragma once

#pragma once

#include "../../util/detect.hpp"
#include "tag.hpp"

namespace p0443::execution::detail
{
void set_value();
struct set_value_impl
{
    template <typename Rx, typename... Args>
    using member_set_value_t = decltype(std::declval<Rx>().set_value(std::declval<Args>()...));

    template <typename Rx, typename... Args>
    using free_set_value_t = decltype(set_value(std::declval<Rx>(), std::declval<Args>()...));

    template <typename Rx, typename... Args>
    static constexpr bool use_member = util::is_detected_v<member_set_value_t, Rx, Args...>;

    template <typename Rx, typename... Args>
    static constexpr bool use_free =
        !use_member<Rx, Args...> && util::is_detected_v<free_set_value_t, Rx, Args...>;

    template <typename Rx, typename... Args>
    static constexpr bool use_indirection = false;

    template <typename Rx, typename... Args>
    void operator()(Rx &&rx, Args &&... values) const {
        tagged_set_value(execution_tag_t<set_value_impl, Rx, Args...>{}, std::forward<Rx>(rx),
                         std::forward<Args>(values)...);
    }

private:
    template <typename Rx, typename... Args>
    void tagged_set_value(tag_member, Rx &&rx, Args &&... values) const {
        rx.set_value(std::forward<Args>(values)...);
    }

    template <typename Rx, typename... Args>
    void tagged_set_value(tag_free, Rx &&rx, Args &&... values) const {
        set_value(std::forward<Rx>(rx), std::forward<Args>(values)...);
    }
};
} // namespace p0443::execution::detail