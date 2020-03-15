
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/submit.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Receiver, class Function>
struct handle_done_receiver : Receiver
{
    Function handler_;
    void set_done() {
        p0443_v2::submit(handler_(), (Receiver &&) * this);
    }
};
template <class Sender, class Function>
struct handle_done_sender
{
    Sender sender_;
    Function handler_;

    template <class Receiver>
    void submit(Receiver &&receiver) {
        using receiver_t = std::decay_t<Receiver>;
        p0443_v2::submit(std::move(sender_),
                         handle_done_receiver<receiver_t, Function>{
                             std::forward<Receiver>(receiver), std::move(handler_)});
    }

    template<class Receiver>
    auto connect(Receiver&& receiver) {
        using receiver_t = p0443_v2::remove_cvref_t<Receiver>;
        return p0443_v2::connect(std::move(sender_), handle_done_receiver<receiver_t, Function>{
                             std::forward<Receiver>(receiver), std::move(handler_)});
    }
};
struct handle_done_cpo
{
    template <class Sender, class Function>
    auto operator()(Sender &&sender, Function &&handler) const {
        using sender_t = std::decay_t<Sender>;
        using function_t = std::decay_t<Function>;
        return handle_done_sender<sender_t, function_t>{std::forward<Sender>(sender),
                                                        std::forward<Function>(handler)};
    }
};
} // namespace detail
constexpr detail::handle_done_cpo handle_done;
} // namespace p0443_v2