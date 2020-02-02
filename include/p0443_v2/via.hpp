#pragma once

#include <optional>
#include <tuple>

#include <p0443_v2/set_done.hpp>
#include <p0443_v2/set_error.hpp>
#include <p0443_v2/set_value.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Scheduler>
struct via_sender
{
    using sender_type = std::decay_t<Sender>;
    using scheduler_type = std::decay_t<Scheduler>;

    template <class NextReceiver>
    struct via_receiver
    {
        template <class Receiver, class... Values>
        struct scheduled_values
        {
            static_assert(sizeof...(Values) > 0, "BUG: scheduled_values should not be used with empty value set");
            using next_receiver_type = std::decay_t<Receiver>;
            using value_type = std::tuple<std::decay_t<Values>...>;

            next_receiver_type receiver_;
            value_type values_;

            template <class Recv, class... Vs>
            explicit scheduled_values(Recv &&recv, Vs &&... vs)
                : receiver_(std::forward<Recv>(recv)), values_(std::forward<Vs>(vs)...) {
            }

            void set_value() {
                auto caller = [this](auto &&... values) {
                    p0443_v2::set_value(this->receiver_, std::forward<decltype(values)>(values)...);
                };
                std::apply(caller, values_);
            }

            void set_done() {
                p0443_v2::set_done(receiver_);
            }

            template <class E>
            void set_error(E &&e) {
                p0443_v2::set_error(receiver_, std::forward<E>(e));
            }
        };

        template <class Receiver>
        struct scheduled_done
        {
            using next_receiver_type = std::decay_t<Receiver>;
            next_receiver_type receiver_;

            template <class Recv>
            explicit scheduled_done(Recv &&recv) : receiver_(std::forward<Recv>(recv)) {
            }

            void set_value() {
                p0443_v2::set_done(receiver_);
            }

            void set_done() {
                p0443_v2::set_done(receiver_);
            }

            template <class E>
            void set_error(E &&e) {
                p0443_v2::set_error(receiver_, std::forward<E>(e));
            }
        };

        template <class Receiver, class Error>
        struct scheduled_error
        {
            using next_receiver_type = std::decay_t<Receiver>;
            using error_type = std::decay_t<Error>;

            next_receiver_type receiver_;
            error_type error_;

            template <class Recv, class Err>
            scheduled_error(Recv &&recv, Err &&err)
                : receiver_(std::forward<Recv>(recv)), error_(std::forward<Err>(err)) {
            }

            void set_value() {
                p0443_v2::set_error(receiver_, error_);
            }

            void set_done() {
                p0443_v2::set_done(receiver_);
            }

            template <class E>
            void set_error(E &&e) {
                p0443_v2::set_error(receiver_, std::forward<E>(e));
            }
        };

        std::decay_t<NextReceiver> next_;

        template<class NR, std::enable_if_t<std::is_constructible_v<std::decay_t<NextReceiver>, NR>>* = nullptr>
        explicit via_receiver(NR &&next): next_(std::forward<NR>(next)) {}

        template <class... Values>
        void set_value(Values &&... values) {
            if constexpr(sizeof...(Values) == 0)
            {
                p0443_v2::submit(scheduler_, next_);
            }
            else
            {
                p0443_v2::submit(scheduler_, scheduled_values<NextReceiver, Values...>(
                                                 next_, std::forward<Values>(values)...));
            }
        }

        void set_done() {
            p0443_v2::submit(scheduler_, scheduled_done<NextReceiver>(next_));
        }

        template <class E>
        void set_error(E &&error) {
            p0443_v2::submit(scheduler_,
                             scheduled_error<NextReceiver, E>(next_, std::forward<E>(error)));
        }
    };

    sender_type target_;
    scheduler_type scheduler_;

    template <class Send, class Sched>
    via_sender(Send &&send, Sched &&sched)
        : target_(std::forward<Send>(send)), scheduler_(std::forward<Sched>(sched)) {
    }

    template<class Receiver>
    void submit(Receiver &&recv) {
        p0443_v2::submit(target_, via_receiver<Receiver>(std::forward<Receiver>(recv)));
    }
};

struct via_fn
{
    template<class Sender, class Scheduler>
    auto operator()(Sender&& sender, Scheduler&& sched) const
    {
        return via_sender<Sender, Scheduler>(std::forward<Sender>(sender), std::forward<Scheduler>(sched));
    }
};
} // namespace detail

constexpr detail::via_fn via;
} // namespace p0443_v2