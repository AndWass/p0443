
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
template<class Receiver>
struct submit_receiver
{
    Receiver next_;
    struct operation_life_extender
    {
        // Holds a type erased pointer to the entire operation state.
        std::unique_ptr<void, void(*)(void*)> operation_{nullptr, +[](void*) {}};
    };

    operation_life_extender* data_ = nullptr;

    template <class R>
    explicit submit_receiver(R &&r)
        : next_(std::forward<R>(r)),
            data_(new operation_life_extender()) {
    }

    template<class...Values>
    void set_value(Values&&...values) {
        p0443_v2::set_value(std::move(next_), std::forward<Values>(values)...);
        delete data_;
    }

    template<class E>
    void set_error(E&& e) {
        p0443_v2::set_error(std::move(next_), std::forward<E>(e));
        delete data_;
    }

    void set_done() {
        p0443_v2::set_done(std::move(next_));
        delete data_;
    }
};
struct submit_impl
{
    template <class Sender, class Receiver>
    void operator()(Sender&& sender, Receiver&& receiver) const {
        using submit_recv_t = submit_receiver<p0443_v2::remove_cvref_t<Receiver>>;
        submit_recv_t op_recv(std::forward<Receiver>(receiver));
        auto *data = op_recv.data_;
        using op_type = operation_type<Sender, submit_recv_t&&>;
        op_type* op_ptr = new op_type(p0443_v2::connect(std::forward<Sender>(sender), std::move(op_recv)));
        data->operation_ = std::unique_ptr<void, void(*)(void*)>(
            op_ptr,
            [](void* p) {
                if(p) {
                    delete static_cast<op_type*>(p);
                }
            }
        );
        p0443_v2::start(*op_ptr);
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