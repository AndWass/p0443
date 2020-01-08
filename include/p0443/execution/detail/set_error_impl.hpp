#pragma once

#pragma once

#include "../../util/detect.hpp"
#include "tag.hpp"

namespace p0443::execution::detail
{
void set_error();
struct set_error_impl
{
    template <typename Rx, typename Err>
    using member_set_error_t = decltype(std::declval<Rx>().set_error(std::declval<Err>()));

    template <typename Rx, typename Err>
    using free_set_error_t = decltype(set_error(std::declval<Rx>(), std::declval<Err>()));

    template <typename Rx, typename Err>
    static constexpr bool use_member = util::is_detected_v<member_set_error_t, Rx, Err>;

    template <typename Rx, typename Err>
    static constexpr bool use_free =
        !use_member<Rx, Err> && util::is_detected_v<free_set_error_t, Rx, Err>;

    template <typename Rx, typename Err>
    static constexpr bool use_indirection = false;

    template <typename Rx, typename Err, std::enable_if_t<use_member<Rx, Err> || use_free<Rx, Err>>* = nullptr>
    void operator()(Rx &&rx, Err &&err) const noexcept {
        tagged_set_error(execution_tag_t<set_error_impl, Rx, Err>{}, std::forward<Rx>(rx),
                         std::forward<Err>(err));
    }

private:
    template <typename Rx, typename Err>
    void tagged_set_error(tag_member, Rx &&rx, Err &&err) const {
        rx.set_error(std::forward<Err>(err));
    }

    template <typename Rx, typename Err>
    void tagged_set_error(tag_free, Rx &&rx, Err &&err) const {
        set_error(std::forward<Rx>(rx), std::forward<Err>(err));
    }
};
} // namespace p0443::execution::detail