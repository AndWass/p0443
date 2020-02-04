
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <tuple>

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/type_traits.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Receiver, class Function>
struct let_receiver : std::decay_t<Receiver>
{
    using receiver_type = std::decay_t<Receiver>;
    using function_type = remove_cvref_t<Function>;

    template <class... Values>
    struct life_extender : std::decay_t<Receiver>
    {
        using storage_type = std::tuple<std::decay_t<Values>...>;
        using base_ = std::decay_t<Receiver>;
        std::shared_ptr<storage_type> data_;

        template <class R, class... Vs>
        explicit life_extender(R &&r, Vs &&... values)
            : base_(std::forward<R>(r)),
              data_(std::make_shared<storage_type>(std::forward<Vs>(values)...)) {
        }

        template <class Fn>
        auto call_with_arguments(Fn &&fn) {
            return std::apply(std::forward<Fn>(fn), *data_);
        }
    };

    function_type function_;

    template <class R, class Fn>
    let_receiver(R &&r, Fn &&fn)
        : receiver_type(std::forward<R>(r)), function_(std::forward<Fn>(fn)) {
    }

    template <class... Values>
    void set_value(Values &&... values) {
        life_extender<Values...> extender((Receiver&&) * this,
                                          std::forward<Values>(values)...);
        // extender will be moved below so ensure we don't do any
        // unspecified evaluation order
        auto next_sender = extender.call_with_arguments(function_);
        p0443_v2::submit(std::move(next_sender), std::move(extender));
    }

    void set_value() {
        p0443_v2::submit(function_(), (receiver_type &&) * this);
    }
};
template <class Sender, class Function>
struct let_sender
{
    using sender_type = remove_cvref_t<Sender>;
    using function_type = remove_cvref_t<Function>;

    sender_type sender_;
    function_type function_;

    template<class S, class F>
    let_sender(S &&s, F &&f): sender_(std::forward<S>(s)), function_(std::forward<F>(f)) {}

    template <class Receiver>
    void submit(Receiver &&receiver) {
        p0443_v2::submit(sender_, let_receiver<Receiver, Function>(std::forward<Receiver>(receiver), std::move(function_)));
    }
};
} // namespace detail
inline constexpr struct let_fn
{
    template <class Sender, class Function>
    auto operator()(Sender && sender, Function && fn) const {
        return detail::let_sender<Sender, Function>{std::forward<Sender>(sender), std::forward<Function>(fn)};
    }
} let;
} // namespace p0443_v2