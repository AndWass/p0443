#pragma once

#include "../../util/detect.hpp"

namespace p0443::execution::detail
{
void submit();
struct submit_impl
{
    template <class Tx, class Rx, class NoRef = std::remove_reference_t<Tx>>
    using member_submit_t = decltype(std::declval<NoRef>().submit(std::declval<Rx>()));

    template <class Tx, class Rx>
    using free_submit_t = decltype(submit(std::declval<Tx>(), std::declval<Rx>()));

    template <class Tx, class Rx>
    static constexpr bool use_member = util::is_detected_v<member_submit_t, Tx, Rx>;

    template <class Tx, class Rx>
    static constexpr bool use_free = !use_member<Tx, Rx> && util::is_detected_v<free_submit_t, Tx, Rx>;

    template<class Tx, class Rx>
    static constexpr bool use_execute = !use_member<Tx, Rx> && !use_free<Tx, Rx>;

    template <class Tx, class Rx>
    std::enable_if_t<use_member<Tx, Rx>> operator()(Tx && tx, Rx && rx) const;

    template <class Tx, class Rx>
    std::enable_if_t<use_free<Tx, Rx>> operator()(Tx && tx, Rx && rx) const;

    /*template <class Tx, class Rx>
    std::enable_if_t<use_execute<Tx, Rx>> operator()(Tx && ex, Rx && fn) const;*/
};
}