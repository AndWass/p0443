
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/type_traits.hpp>
#include <p0443_v2/connect.hpp>

#include <p0443_v2/sender_traits.hpp>

namespace p0443_v2
{
namespace detail
{
template <class SenderFactory, class Predicate>
struct submit_while_sender
{
    using sender_factory_type = p0443_v2::remove_cvref_t<SenderFactory>;
    using predicate_type = p0443_v2::remove_cvref_t<Predicate>;

    using sender_type = std::invoke_result_t<sender_factory_type>;

    template <template <class...> class Tuple, template <class...> class Variant>
    using value_types =
        typename p0443_v2::sender_traits<sender_type>::template value_types<Tuple, Variant>;

    template <template <class...> class Variant>
    using error_types =
        typename p0443_v2::sender_traits<sender_type>::template error_types<Variant>;

    constexpr static bool sends_done = p0443_v2::sender_traits<sender_type>::sends_done;

    template <class Receiver>
    struct receiver_t
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;

        sender_factory_type sender_;
        predicate_type predicate_;
        receiver_type target_;

        template <class S, class P, class R>
        receiver_t(S &&sender, P &&predicate, R &&target)
            : sender_(std::forward<S>(sender)), predicate_(std::forward<P>(predicate)),
              target_(std::forward<R>(target)) {
        }

        template <class... Values>
        void set_value(Values &&... values) {
            try {
                bool predicate_result = predicate_(values...);

                if (predicate_result) {
                    p0443_v2::submit(sender_(), receiver_t<Receiver>(*this));
                }
                else {
                    p0443_v2::set_value(target_, std::forward<Values>(values)...);
                }
            }
            catch (...) {
                p0443_v2::set_error(target_, std::current_exception());
            }
        }

        void set_done() {
            p0443_v2::set_done(target_);
        }

        template <class Error>
        void set_error(Error &&error) {
            p0443_v2::set_error(target_, std::forward<Error>(error));
        }
    };

    sender_factory_type sender_;
    predicate_type predicate_;

    template <class S, class P>
    submit_while_sender(S &&sender, P &&predicate)
        : sender_(std::forward<S>(sender)), predicate_(std::forward<P>(predicate)) {
    }

    template<class Receiver>
    auto connect(Receiver &&receiver) {
        return p0443_v2::connect(sender_(), receiver_t<Receiver>(sender_, predicate_, std::forward<Receiver>(receiver)));
    }
};

struct submit_while_fn
{
    template <class SenderFactory, class Predicate>
    auto operator()(SenderFactory && sender_factory, Predicate && predicate) const {
        static_assert(std::is_invocable<SenderFactory>::value,
                      "SenderFactory must be of the signature sender fn()");
        return submit_while_sender<SenderFactory, Predicate>(
            std::forward<SenderFactory>(sender_factory), std::forward<Predicate>(predicate));
    }
};
} // namespace detail

constexpr detail::submit_while_fn submit_while;
} // namespace p0443_v2