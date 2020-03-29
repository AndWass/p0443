
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tag_invoke.hpp"
#include "type_traits.hpp"

#include <memory>

#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>

namespace p0443_v2
{
namespace detail
{
inline void do_nothing_deleter(void*) {}
struct submit_impl
{
    template<class Sender, class Receiver>
    struct submit_receiver
    {
        struct wrap
        {
            submit_receiver<Sender, Receiver> *owner_;
            template<class...Values>
            void set_value(Values&&...values) && {
                p0443_v2::set_value(std::move(owner_->next_), (Values&&)values...);
                delete owner_;
            }

            template<class E>
            void set_error(E&& e) && {
                p0443_v2::set_error(std::move(owner_->next_), (E&&)e);
                delete owner_;
            }

            void set_done() && noexcept {
                p0443_v2::set_done(std::move(owner_->next_));
            }
        };

        p0443_v2::remove_cvref_t<Receiver> next_;
        p0443_v2::operation_type<Sender, wrap> state_;
        submit_receiver(Sender&& s, Receiver&& r): next_((Receiver&&)r), state_(p0443_v2::connect((Sender&&)s, wrap{this})) {}
    };
    template <class Sender, class Receiver>
    void operator()(Sender&& sender, Receiver&& receiver) const {
        p0443_v2::start((new submit_receiver<Sender, Receiver>((Sender&&)sender, (Receiver&&)receiver))->state_);
    }
};
} // namespace detail
constexpr detail::submit_impl submit;

namespace tag
{
template <class Sender, class Receiver>
void
tag_invoke(Sender &&sender, p0443_v2::tag::submit_t, Receiver &&receiver) {
    ::p0443_v2::submit(std::forward<Sender>(sender), std::forward<Receiver>(receiver));
}
} // namespace tag
} // namespace p0443_v2