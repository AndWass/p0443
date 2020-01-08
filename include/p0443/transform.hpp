#pragma once

#include "execution/execution.hpp"
#include "util/type_traits.hpp"
#include "util/utility.hpp"

#include <utility>
#include <tuple>

namespace p0443
{
namespace detail
{
template <class Receiver, class Function>
struct transformed_receiver
{
    using receiver_type = util::remove_cvref_t<Receiver>;
    using function_type = util::remove_cvref_t<Function>;

    receiver_type stored_receiver_;
    function_type fun_;

    template <class... Values>
    void set_value(Values &&... values) {
        execution::set_value(stored_receiver_, values...);
        fun_(values...);
    }

    void set_done() {
        execution::set_done(stored_receiver_);
    }

    template <class E>
    void set_error(E &&err) {
        execution::set_error(stored_receiver_, std::forward<E>(err));
    }
};

template <class Sender, class Function>
struct transformed_sender
{
    using function_type = util::remove_cvref_t<Function>;
    using sender_type = util::remove_cvref_t<Sender>;

    template <class Fn>
    void execute(Fn &&fn) {
        execution::execute((sender_type&&)sender_, std::forward<Fn>(fn));
    }

    template <class Receiver>
    void submit(Receiver &&rx) {
        execution::submit(sender_, transformed_receiver<Receiver, Function>{
                                       std::forward<Receiver>(rx), util::decay_copy(fun_)});
    }
    sender_type sender_;
    function_type fun_;
};
} // namespace detail

template <class Sender, class Function>
auto transform(Sender &&sender, Function &&fn) {
    return detail::transformed_sender<Sender, Function>{std::forward<Sender>(sender),
                                                        util::decay_copy(fn)};
}
} // namespace p0443