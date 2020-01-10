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

    template<class Fn>
    sender_impl(Fn &&fn): submit_(std::forward<Fn>(fn)) {}

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