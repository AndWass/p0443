#pragma once

#include "../../util/detect.hpp"

namespace p0443::execution::detail
{
void submit();
struct submit_impl
{
    template <typename Tx, typename Rx, typename NoRef = std::remove_reference_t<Tx>>
    using member_submit_t = decltype(std::declval<NoRef>().submit(std::declval<Rx>()));

    template <typename Tx, typename Rx>
    using free_submit_t = decltype(submit(std::declval<Tx>(), std::declval<Rx>()));

    template <typename Tx, typename Rx>
    static constexpr bool use_member = util::is_detected_v<member_submit_t, Tx, Rx>;

    template <typename Tx, typename Rx>
    static constexpr bool use_free =
        !use_member<Tx, Rx> && util::is_detected_v<free_submit_t, Tx, Rx>;

    template <typename Tx, typename Rx>
    static constexpr bool use_indirection = !use_member<Tx, Rx> && !use_free<Tx, Rx>;

    template <typename Tx, typename Rx>
    void operator()(Tx &&tx, Rx &&rx) const {
        tagged_submit(execution_tag_t<submit_impl, Tx, Rx>{}, std::forward<Tx>(tx),
                      std::forward<Rx>(rx));
    }

private:
    template <typename Tx, typename Rx>
    void tagged_submit(tag_member, Tx &&tx, Rx &&rx) const;

    template <typename Tx, typename Rx>
    void tagged_submit(tag_free, Tx &&tx, Rx &&rx) const;

    template <typename Tx, typename Rx>
    void tagged_submit(tag_indirection, Tx &&tx, Rx &&rx) const;
};
} // namespace p0443::execution::detail