#pragma once

#include <exception>

#include "../../util/type_traits.hpp"
#include "execute_impl.hpp"
#include "submit_impl.hpp"

namespace p0443::execution::detail
{
template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_member, Tx &&tx, Rx &&rx) const {
    tx.submit(std::forward<Rx>(rx));
}

template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_free, Tx &&tx, Rx &&rx) const {
    submit(std::forward<Tx>(tx), std::forward<Rx>(rx));
}

template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_indirection, Tx &&tx, Rx &&rx) const {
    execute_impl{}(std::forward<Tx>(tx), as_invocable<util::remove_cvref_t<Rx>>(std::forward<Rx>(rx)));
}
} // namespace p0443::execution::detail