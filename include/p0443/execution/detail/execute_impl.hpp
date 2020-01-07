#pragma once

#include "../../util/detect.hpp"
#include "tag.hpp"

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
    static constexpr bool use_free =
        !use_member<Ex, Fn> && util::is_detected_v<free_execute_t, Ex, Fn>;

    template <class Ex, class Fn>
    static constexpr bool use_indirection = !use_free<Ex, Fn> && !use_member<Ex, Fn>;

    /*template <typename Ex, typename Fn>
    void operator()(Ex && ex, Fn && fn, std::enable_if_t<use_member<Ex, Fn>>* = nullptr) const;*/

    template <typename Ex, typename Fn>
    void operator()(Ex &&ex, Fn &&fn) const {
        this->tagged_execute(std::forward<Ex>(ex), std::forward<Fn>(fn),
                             execution_tag_t<execute_impl, Ex, Fn>{});
    }

private:
    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_member) const;

    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_free) const;

    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_indirection) const;
};
} // namespace p0443::execution::detail