
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "p0443_v2/set_done.hpp"
#include "p0443_v2/set_value.hpp"
#include "p0443_v2/type_traits.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

namespace p0443_v2::asio
{
namespace timer
{
    template <class TimerType>
    struct wait_until
    {
        using timer_type = TimerType;
        using time_point = typename timer_type::time_point;
        timer_type *timer_;
        time_point expiration_time_;

        template<template<class...> class Tuple, template<class...> class Variant>
        using value_types = Variant<Tuple<>>;

        template<template<class...> class Variant>
        using error_types = Variant<std::exception_ptr>;

        static constexpr bool sends_done = true;

        template <class Receiver>
        auto connect(Receiver &&receiver) {
            struct operation
            {
                timer_type *timer_;
                time_point expiration_time_;
                p0443_v2::remove_cvref_t<Receiver> receiver_;

                void start() {
                    timer_->expires_at(expiration_time_);
                    timer_->async_wait(
                        [rx = std::move(receiver_)](const boost::system::error_code &ec) mutable {
                            if (ec == boost::asio::error::operation_aborted) {
                                p0443_v2::set_done(std::move(rx));
                            }
                            else {
                                p0443_v2::set_value(std::move(rx));
                            }
                        });
                }
            };
            return operation{timer_, expiration_time_, std::forward<Receiver>(receiver)};
        }

        wait_until(TimerType &timer, time_point expiration_time)
            : timer_(&timer), expiration_time_(expiration_time) {
        }
    };

    template <class TimerType>
    struct wait_for
    {
        using timer_type = TimerType;
        using duration = typename timer_type::duration;

        template<template<class...> class Tuple, template<class...> class Variant>
        using value_types = Variant<Tuple<>>;

        template<template<class...> class Variant>
        using error_types = Variant<std::exception_ptr>;

        static constexpr bool sends_done = true;

        timer_type *timer_;
        duration duration_;

        wait_for(TimerType &timer, duration dur)
            : timer_(&timer), duration_(dur) {
        }

        template<class Receiver>
        struct operation
        {
            timer_type *timer_;
            duration duration_;
            Receiver receiver_;

            void start() {
                timer_->expires_after(duration_);
                timer_->async_wait(
                    [rx = std::move(receiver_)](const boost::system::error_code &ec) mutable {
                        if (ec == boost::asio::error::operation_aborted) {
                            p0443_v2::set_done(std::move(rx));
                        }
                        else {
                            p0443_v2::set_value(std::move(rx));
                        }
                    });
            }
        };

        template <class Receiver>
        auto connect(Receiver &&receiver) {
            return operation<p0443_v2::remove_cvref_t<Receiver>>{timer_, duration_, std::forward<Receiver>(receiver)};
        }
    };
};
} // namespace p0443_v2::asio
