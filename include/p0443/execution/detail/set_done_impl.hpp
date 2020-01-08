#pragma once

#pragma once

#include "../../util/detect.hpp"
#include "tag.hpp"

namespace p0443::execution::detail
{
void set_done();
struct set_done_impl
{
    template <typename Rx>
    using member_set_done_t = decltype(std::declval<Rx>().set_done());

    template <typename Rx>
    using free_set_done_t = decltype(set_done(std::declval<Rx>()));

    template <typename Rx>
    static constexpr bool use_member = util::is_detected_v<member_set_done_t, Rx>;

    template <typename Rx>
    static constexpr bool use_free = !use_member<Rx> && util::is_detected_v<free_set_done_t, Rx>;

    template <typename Rx>
    static constexpr bool use_indirection = false;

    template <typename Rx, std::enable_if_t<use_member<Rx> || use_free<Rx>>* = nullptr>
    void operator()(Rx &&rx) const noexcept {
        tagged_set_done(execution_tag_t<set_done_impl, Rx>{}, std::forward<Rx>(rx));
    }

private:
    template <typename Rx>
    void tagged_set_done(tag_member, Rx &&rx) const {
        rx.set_done();
    }

    template <typename Rx>
    void tagged_set_done(tag_free, Rx &&rx) const {
        set_done(std::forward<Rx>(rx));
    }
};
} // namespace p0443::execution::detail