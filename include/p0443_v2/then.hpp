
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <p0443_v2/submit.hpp>

namespace p0443_v2
{
namespace detail
{
template<class Receiver, class Function>
struct then_receiver: Receiver
{
    Function function_;

    template<class...Values>
    void set_value(Values&&...values) {
        p0443_v2::submit(function_(values...), (Receiver&&)*this);
    }
};

template<class Sender, class Function>
struct then_sender: Sender
{
    Function function_;

    template<class Receiver>
    void submit(Receiver&& recv) {
        using receiver_t = std::decay_t<Receiver>;
        p0443_v2::submit((Sender&&)*this, then_receiver<receiver_t, Function>{(Receiver&&)recv, std::move(function_)});
    }
};

struct then_cpo
{
    template<class Sender, class Function>
    auto operator()(Sender&& sender, Function&& fn) const {
        using sender_t = std::decay_t<Sender>;
        using function_t = std::decay_t<Function>;

        return then_sender<sender_t, function_t>{std::forward<Sender>(sender), std::forward<Function>(fn)};
    }
};
}
constexpr detail::then_cpo then;
}