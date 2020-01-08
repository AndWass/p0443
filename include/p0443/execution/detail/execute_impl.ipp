#pragma once

#include <type_traits>

#include "../../util/utility.hpp"
#include "../../util/type_traits.hpp"
#include "execute_impl.hpp"

namespace p0443::execution::detail
{
template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_member) const {
    ex.execute(std::forward<Fn>(fn));
}

template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_free) const {
    execute(std::forward<Ex>(ex), std::forward<Fn>(fn));
}

template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_indirection) const {
    static_assert(std::is_invocable_v<submit_impl, Ex, as_receiver<Fn>>, "Bad executor type used");

    submit_impl{}(std::forward<Ex>(ex), as_receiver<Fn>{std::forward<Fn>(fn)});
}
} // namespace p0443::execution::detail