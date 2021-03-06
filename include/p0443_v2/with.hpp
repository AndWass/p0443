
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/just.hpp>
#include <p0443_v2/let.hpp>

#include <tuple>

namespace p0443_v2
{
inline constexpr struct with_fn
{
    template<class Function, class...Values>
    auto operator()(Function &&function, Values&&...values) const {
        return p0443_v2::let(p0443_v2::just(std::forward<Values>(values)...), std::forward<Function>(function));
    }
} with;
} // namespace p0443_v2