
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
    template<class Receiver>
    struct submit_receiver
    {
        struct operation_state_holder
        {
            // Holds a type erased pointer to the entire operation state.
            std::unique_ptr<void, void(*)(void*)> operation_{nullptr, +[](void*) {}};
        };

        Receiver next_;
        operation_state_holder* data_ = nullptr;

        template <class R, class... Vs>
        explicit submit_receiver(R &&r)
            : next_(std::forward<R>(r)),
              data_(new operation_state_holder) {
        }

        template<class...Values>
        void set_value(Values&&...values) {
            p0443_v2::set_value(std::move(next_), std::forward<Values>(values)...);
            if(data_) {
                delete data_;
            }
        }

        template<class E>
        void set_error(E&& e) {
            p0443_v2::set_error(std::move(next_), std::forward<E>(e));
            if(data_) {
                delete data_;
            }
        }

        void set_done() {
            p0443_v2::set_done(std::move(next_));
            if(data_) {
                delete data_;
            }
        }
    };
    template <class Sender, class Receiver>
    void operator()(Sender&& sender, Receiver&& receiver) const {
        using wrapped_receiver_t = submit_receiver<p0443_v2::remove_cvref_t<Receiver>>;
        using submit_op_t = p0443_v2::operation_type<Sender, wrapped_receiver_t&&>;
        wrapped_receiver_t wrapped_receiver(std::forward<Receiver>(receiver));
        auto *data_storage = wrapped_receiver.data_;
        auto *next_op = new submit_op_t(p0443_v2::connect(std::forward<Sender>(sender), std::move(wrapped_receiver)));
        data_storage->operation_ = decltype(data_storage->operation_)(next_op, +[](void* p) {
            delete (static_cast<submit_op_t*>(p));
        });
        p0443_v2::start(*next_op);
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