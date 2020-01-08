#pragma once

#include "detail/submit_impl.hpp"
#include "execution.hpp"
#include "p0443/util/type_traits.hpp"
#include "receiver_traits.hpp"

#include <exception>

namespace p0443::execution
{
namespace detail
{
template <class Sender, class Receiver>
using is_sender_to_impl = std::is_invocable<detail::submit_impl, Sender &&, Receiver &&>;
} // namespace detail

class sink_receiver
{
public:
    void set_value(auto &&...) {
    }
    [[noreturn]] void set_error(auto &&) noexcept {
        std::terminate();
    }
    void set_done() noexcept {
    }
};

template <class Sender>
using is_sender =
    std::conjunction <
    std::is_move_constructible<util::remove_cvref_t<Sender>,
                               detail::is_sender_to_impl<Sender, sink_receiver>>;

template <class Sender, class Receiver>
using is_sender_to = std::conjunction<is_sender<Sender>, is_receiver<Receiver>,
                                      detail::is_sender_to_impl<Sender, Receiver>>;
} // namespace p0443::execution