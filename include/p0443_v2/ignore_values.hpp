
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <p0443_v2/transform.hpp>

namespace p0443_v2
{
inline constexpr struct ignore_values_fn
{
    template<class Sender>
    auto operator()(Sender&& sender) const noexcept {
        return p0443_v2::transform(std::forward<Sender>(sender), [](auto&&...) {
        });
    }
} ignore_values;
}