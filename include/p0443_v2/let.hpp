
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <tuple>
#include <variant>

#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/connect.hpp>
#include <p0443_v2/type_traits.hpp>
#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/start.hpp>

#include <boost/mp11/algorithm.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Receiver, class Function>
struct let_receiver : Receiver
{
    using receiver_type = std::decay_t<Receiver>;
    using function_type = remove_cvref_t<Function>;

    template <class ValueTuple>
    struct life_extender : Receiver
    {
        using base_ = p0443_v2::remove_cvref_t<Receiver>;
        struct life_extended_data
        {
            ValueTuple data_;
            std::shared_ptr<void> operation_;

            template<class...Vs>
            life_extended_data(Vs&&...vs): data_(std::forward<Vs>(vs)...) {}
        };
        std::shared_ptr<life_extended_data> data_;

        template <class R, class... Vs>
        explicit life_extender(R &&r, Vs &&... values)
            : base_(std::forward<R>(r)),
              data_(std::make_shared<life_extended_data>(std::forward<Vs>(values)...)) {
        }

        template <class Fn>
        auto call_with_arguments(Fn &&fn) {
            return std::apply(std::forward<Fn>(fn), data_->data_);
        }
    };

    template <>
    struct life_extender<std::tuple<>> : Receiver
    {
        template <class R>
        explicit life_extender(R &&r) : Receiver(std::forward<R>(r)) {
        }

        template <class Fn>
        auto call_with_arguments(Fn &&fn) {
            return fn();
        }
    };

    function_type function_;

    template <class R, class Fn>
    let_receiver(R &&r, Fn &&fn)
        : receiver_type(std::forward<R>(r)), function_(std::forward<Fn>(fn)) {
    }

    template <class... Values>
    void set_value(Values &&... values) {
        using life_extender_type = life_extender<std::tuple<p0443_v2::remove_cvref_t<Values>...>>;
        life_extender_type extender(
            (Receiver &&) * this, std::forward<Values>(values)...);

        // Get the pointer to the life-extended data
        // The connected operation will be stored
        // in a shared pointer in here, and that way will
        // be managed by the life extender
        auto* data_ptr = extender.data_.get();

        // extender will be moved below so ensure we don't do any
        // unspecified evaluation order
        auto next_sender = extender.call_with_arguments(function_);
        using operations_type = p0443_v2::operation_type<decltype(next_sender), life_extender_type>;
        auto *op_ptr = new operations_type(p0443_v2::connect(std::move(next_sender), std::move(extender)));
        data_ptr->operation_.reset(op_ptr, +[](void* p) {
            delete static_cast<operations_type*>(p);
        });
        p0443_v2::start(*op_ptr);
    }
};
template <class Sender, class Function>
struct let_sender
{
    using sender_type = remove_cvref_t<Sender>;
    using function_type = remove_cvref_t<Function>;

    sender_type sender_;
    function_type function_;

    template <template <class...> class Variant>
    using function_result_types =
        p0443_v2::function_result_types<Variant, function_type, sender_type>;

    template <template <class...> class Tuple, template <class...> class Variant>
    struct value_types_extractor
    {
        template <class ST>
        using extractor =
            typename p0443_v2::sender_traits<ST>::template value_types<Tuple, Variant>;

        using sender_value_types =
            boost::mp11::mp_transform<extractor, function_result_types<Variant>>;

        template <class T1, class T2>
        using concat = p0443_v2::concat_value_types<Tuple, Variant, T1, T2>;

        using folded_sender_value_types =
            boost::mp11::mp_fold<sender_value_types, boost::mp11::mp_first<sender_value_types>,
                                 concat>;
    };

    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types = typename value_types_extractor<Tuple, Variant>::folded_sender_value_types;

    template <template <class...> class Variant>
    using error_types =
        typename p0443_v2::sender_traits<sender_type>::template error_types<Variant>;

    static constexpr bool sends_done = p0443_v2::sender_traits<sender_type>::sends_done;

    template <class S, class F>
    let_sender(S &&s, F &&f) : sender_(std::forward<S>(s)), function_(std::forward<F>(f)) {
    }

    template <class Receiver>
    void submit(Receiver &&receiver) {
        p0443_v2::submit(sender_,
                         let_receiver<sender_type, p0443_v2::remove_cvref_t<Receiver>, Function>(
                             std::forward<Receiver>(receiver), std::move(function_)));
    }

    template <class Receiver>
    auto connect(Receiver &&receiver) {
        return p0443_v2::connect(
            sender_, let_receiver<sender_type, p0443_v2::remove_cvref_t<Receiver>, Function>(
                         std::forward<Receiver>(receiver), std::move(function_)));
    }
};
} // namespace detail
inline constexpr struct let_fn
{
    template <class Sender, class Function>
    auto operator()(Sender && sender, Function && fn) const {
        return detail::let_sender<Sender, Function>{std::forward<Sender>(sender),
                                                    std::forward<Function>(fn)};
    }
} let;
} // namespace p0443_v2