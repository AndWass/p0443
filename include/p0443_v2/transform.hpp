#pragma once
#include <p0443_v2/type_traits.hpp>
#include <p0443_v2/submit.hpp>

#include <exception>
#include <utility>

#include <p0443_v2/make_receiver.hpp>

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Function>
struct transform_op
{
    using sender_type = p0443_v2::remove_cvref_t<Sender>;
    using function_type = p0443_v2::remove_cvref_t<Function>;

    template <class Receiver>
    struct receiver
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;
        receiver_type next_;
        function_type fn_;
    private:
        struct tag_helper
        {
            template <class Receiver, class Function, class... Values>
            std::enable_if_t<p0443_v2::is_receiver_for_values_v<Receiver>>
            operator()(std::true_type, Receiver&& next, Function&& fn, Values&&...values)
            {
                try {
                    fn(std::forward<Values>(values)...);
                    p0443_v2::set_value((receiver_type&&)next);
                }
                catch (...) {
                    p0443_v2::set_error(next, std::current_exception());
                }
            }

            template <class Receiver, class Function, class... Values>
            std::enable_if_t<p0443_v2::is_receiver_for_values_v<Receiver, std::invoke_result_t<Function, Values...>>>
            operator()(std::false_type, Receiver&& next, Function&& fn, Values&&...values)
            {
                try {
                    p0443_v2::set_value((receiver_type&&)next, fn(std::forward<Values>(values)...));
                }
                catch (...) {
                    p0443_v2::set_error(next, std::current_exception());
                }
            }
        };
        public:

        template <class Rx, class Fn>
        receiver(Rx &&rx, Fn &&fn) : next_(std::forward<Rx>(rx)), fn_(fn) {
        }

        template <class... Values>
        std::enable_if_t<
            std::is_invocable_v<tag_helper,
                std::is_void<std::invoke_result_t<function_type, Values...>>,
                receiver_type&,
                function_type&,
                Values...
            >
        >
        set_value(Values &&... values) {
            tag_helper{}(std::is_void<std::invoke_result_t<function_type, Values...>>{},
                next_, fn_, std::forward<Values>(values)...);
        }

        void set_done() {
            p0443_v2::set_done(next_);
        }

        template <class E>
        void set_error(E &&e) {
            p0443_v2::set_error(next_, e);
        }
    };

    sender_type sender_;
    function_type function_;

    template <class Receiver>
    void submit(Receiver &&rx) {
        p0443_v2::submit((sender_type &&) sender_,
                         receiver<Receiver>(std::forward<Receiver>(rx), function_));
    }

    template <class S, class Fn>
    transform_op(S &&sender, Fn &&fn)
        : sender_(std::forward<S>(sender)), function_(std::forward<Fn>(fn)) {
    }
};

template <class Sender, class Function>
struct transform_before_op
{
    using sender_type = p0443_v2::remove_cvref_t<Sender>;
    using function_type = p0443_v2::remove_cvref_t<Function>;

    template <class Receiver>
    struct receiver
    {
        using receiver_type = p0443_v2::remove_cvref_t<Receiver>;
        receiver_type next_;
        function_type fn_;

        template <class Rx, class Fn>
        receiver(Rx &&rx, Fn &&fn) : next_(std::forward<Rx>(rx)), fn_(fn) {
        }

        template <class... Values>
        std::enable_if_t<p0443_v2::is_receiver_for_values_v<Receiver, Values...>>
        set_value(Values &&... values) {
            try {
                fn_(values...);
                p0443_v2::set_value((receiver_type&&)next_, values...);
            }
            catch (...) {
                p0443_v2::set_error(next_, std::current_exception());
            }
        }

        void set_done() {
            p0443_v2::set_done(next_);
        }

        template <class E>
        void set_error(E &&e) {
            p0443_v2::set_error(next_, e);
        }
    };

    sender_type sender_;
    function_type function_;

    template <class Receiver>
    void submit(Receiver &&rx) {
        p0443_v2::submit((sender_type &&) sender_,
                         receiver<Receiver>(std::forward<Receiver>(rx), function_));
    }

    template <class S, class Fn>
    transform_before_op(S &&sender, Fn &&fn)
        : sender_(std::forward<S>(sender)), function_(std::forward<Fn>(fn)) {
    }
};

struct transform_fn
{
    template <class Sender, class Function>
    auto operator()(Sender &&sender, Function &&fn) const
    {
        return ::p0443_v2::detail::transform_op<Sender, Function>(std::forward<Sender>(sender),
                                                  std::forward<Function>(fn));
    }
};
} // namespace detail

constexpr ::p0443_v2::detail::transform_fn transform{};

/*template <class Sender, class Function>
auto transform(Sender &&sender, Function &&fn) {
    return detail::transform_op<Sender, Function>(std::forward<Sender>(sender),
                                                  std::forward<Function>(fn));
}

template <class Sender, class Function>
auto transform_before(Sender &&sender, Function &&fn) {
    return detail::transform_before_op<Sender, Function>(std::forward<Sender>(sender),
                                                  std::forward<Function>(fn));
}*/
} // namespace p0443_v2
