#pragma once

#pragma once

#include "../../util/detect.hpp"
#include "../../util/type_traits.hpp"
#include "tag.hpp"

namespace p0443::execution::detail
{
void schedule();
struct schedule_impl
{
    template <typename Scheduler>
    using member_schedule_t = decltype(std::declval<Scheduler>().schedule());

    template <typename Scheduler>
    using free_schedule_t = decltype(schedule(std::declval<Scheduler>()));

    template <typename Scheduler>
    static constexpr bool use_member = util::is_detected_v<member_schedule_t, Scheduler>;

    template <typename Scheduler>
    static constexpr bool use_free =
        !use_member<Scheduler> && util::is_detected_v<free_schedule_t, Scheduler>;

    template <typename Scheduler>
    static constexpr bool use_indirection = !use_member<Scheduler> && !use_free<Scheduler>;

    template <typename Scheduler>
    decltype(auto) operator()(Scheduler &&scheduler) const {
        tagged_schedule(execution_tag_t<schedule_impl, Scheduler>{},
                        std::forward<Scheduler>(scheduler));
    }

private:
    template <typename Scheduler>
    decltype(auto) tagged_schedule(tag_member, Scheduler &&scheduler) const {
        return scheduler.schedule();
    }

    template <typename Scheduler>
    decltype(auto) tagged_schedule(tag_free, Scheduler &&scheduler) const {
        return schedule(std::forward<Scheduler>(scheduler));
    }

    template <typename Scheduler>
    util::remove_cvref_t<Scheduler> tagged_schedule(tag_indirection, Scheduler &&scheduler) const {
        return scheduler;
    }
};
} // namespace p0443::execution::detail