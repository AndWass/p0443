#pragma once

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/type_traits.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Predicate>
struct submit_while_sender
{
    using sender_type = p0443_v2::remove_cvref_t<Sender>;
    using predicate_type = p0443_v2::remove_cvref_t<Predicate>;

    template <class Receiver>
    struct receiver_t
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;

        sender_type sender_;
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
                    p0443_v2::submit(sender_, receiver_t<Receiver>(*this));
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

    sender_type sender_;
    predicate_type predicate_;

    template <class S, class P>
    submit_while_sender(S &&sender, P &&predicate)
        : sender_(std::forward<S>(sender)), predicate_(std::forward<P>(predicate)) {
    }

    template <class Receiver>
    void submit(Receiver &&receiver) {
        p0443_v2::submit(sender_,
                         receiver_t<Receiver>(sender_, predicate_,
                                            std::forward<Receiver>(receiver)));
    }
};

struct submit_while_fn
{
    template <class Sender, class Predicate>
    auto operator()(Sender && sender, Predicate && predicate) const {
        return submit_while_sender<Sender, Predicate>(std::forward<Sender>(sender),
                                                      std::forward<Predicate>(predicate));
    }
};
} // namespace detail

constexpr detail::submit_while_fn submit_while;
} // namespace p0443_v2