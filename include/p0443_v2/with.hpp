#pragma once

#include <p0443_v2/just.hpp>
#include <p0443_v2/let.hpp>

#include <tuple>

namespace p0443_v2
{
inline constexpr struct with_fn
{
    template <class...Values>
    auto operator()(Values &&... values) const {
        static_assert(sizeof...(values) >= 2, "At least 2 arguments must be provided");
        auto forwarding_tuple = std::forward_as_tuple(std::forward<Values>(values)...);
        return apply_helper(std::move(forwarding_tuple), std::make_index_sequence<std::tuple_size_v<decltype(forwarding_tuple)>-1>{});
    }

    template<class Tuple, std::size_t...I>
    auto apply_helper(Tuple&& tuple, std::index_sequence<I...>) const {
        return p0443_v2::let(p0443_v2::just(std::get<I>(std::forward<Tuple>(tuple))...), std::get<std::tuple_size_v<Tuple>-1>(tuple));
    }
} with;
} // namespace p0443_v2