
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/mp11/algorithm.hpp>
#include <type_traits>

namespace p0443_v2
{
namespace make_sender_detail
{
template<class SubmitFn>
struct sender_impl
{
    using submit_fn_type = std::decay_t<SubmitFn>;
    submit_fn_type submit_;

    template<class Fn, std::enable_if_t<std::is_constructible_v<submit_fn_type, Fn>>* = nullptr>
    explicit sender_impl(Fn &&fn): submit_(std::forward<Fn>(fn)) {}

    template<class Receiver>
    void submit(Receiver &&recv) {
        submit_(std::forward<Receiver>(recv));
    }
};

struct make_sender_fn
{
    template <class Submit>
    auto operator()(Submit&& submitfn) const {
        return sender_impl<Submit>(std::forward<Submit>(submitfn));
    }
};
} // namespace make_sender_detail

constexpr make_sender_detail::make_sender_fn make_sender;
} // namespace p0443_v2