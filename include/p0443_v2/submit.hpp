#pragma once

#include "tag_invoke.hpp"
#include "type_traits.hpp"

#include <exception>
#include <optional>
#include <utility>

#include "set_done.hpp"
#include "set_error.hpp"
#include "set_value.hpp"

namespace p0443_v2
{
namespace detail
{
template <class Sender, class Receiver>
using has_submit_member_detector =
    decltype(std::declval<Sender>().submit(std::declval<Receiver>()));

void submit();
template <class Sender, class Receiver>
using has_submit_free_detector = decltype(submit(std::declval<Sender>(), std::declval<Receiver>()));

template <class Receiver>
struct as_invocable
{
private:
    using receiver_type = p0443_v2::remove_cvref_t<Receiver>;
    std::optional<receiver_type> r_{};

    template <typename R>
    void try_init_(R r) {
        try {
            r_.emplace((decltype(r) &&)r);
        }
        catch (...) {
            p0443_v2::set_error(r, std::current_exception());
        }
    }

public:
    explicit as_invocable(receiver_type &&r) {
        try_init_(std::move_if_noexcept(r));
    }
    explicit as_invocable(const receiver_type &r) {
        try_init_(r);
    }
    as_invocable(as_invocable &&other) {
        if (other.r_) {
            try_init_(std::move_if_noexcept(*other.r_));
            other.r_.reset();
        }
    }
    ~as_invocable() {
        if (r_)
            ::p0443_v2::set_done(*r_);
    }
    void operator()() {
        try {
            ::p0443_v2::set_value(*r_);
        }
        catch (...) {
            ::p0443_v2::set_error(*r_, std::current_exception());
        }
        r_.reset();
    }
};

struct submit_impl
{
    template <class Sender, class Receiver>
    using use_member = std::conjunction<
        p0443_v2::is_receiver<Receiver>,
        p0443_v2::is_detected<detail::has_submit_member_detector, Sender, Receiver>
    >;

    template <class Sender, class Receiver>
    using use_free_function = std::conjunction<
        p0443_v2::is_receiver<Receiver>,
        std::negation<p0443_v2::is_detected<detail::has_submit_member_detector, Sender, Receiver>>,
        p0443_v2::is_detected<detail::has_submit_free_detector, Sender, Receiver>>;

    template <class Sender, class Receiver>
    using use_execute = std::conjunction<
        p0443_v2::is_receiver_of<Receiver>,
        std::negation<use_member<Sender, Receiver>>,
                                         std::negation<use_free_function<Sender, Receiver>>,
                                         ::p0443_v2::is_executor<Sender>
                                         >;

    template <class Sender, class Receiver>
    std::enable_if_t<use_member<Sender, Receiver>::value> operator()(Sender && sender,
                                                                     Receiver && receiver) const {
        sender.submit(std::forward<Receiver>(receiver));
    }

    template <class Sender, class Receiver>
    std::enable_if_t<use_free_function<Sender, Receiver>::value>
    operator()(Sender && sender, Receiver && receiver) const {
        submit(std::forward<Sender>(sender), std::forward<Receiver>(receiver));
    }

    template <class Sender, class Receiver>
    std::enable_if_t<use_execute<Sender, Receiver>::value> operator()(Sender && sender,
                                                                      Receiver && receiver) const {
        ::p0443_v2::tag_invoke(std::forward<Sender>(sender), ::p0443_v2::tag::execute,
                               as_invocable<Receiver>(std::forward<Receiver>(receiver)));
    }
};
} // namespace detail
constexpr detail::submit_impl submit;

namespace tag
{
template <class Sender, class Receiver>
std::enable_if_t<std::disjunction_v<detail::submit_impl::use_member<Sender, Receiver>,
                                    detail::submit_impl::use_free_function<Sender, Receiver>,
                                    detail::submit_impl::use_execute<Sender, Receiver>>>
tag_invoke(Sender &&sender, p0443_v2::tag::submit_t, Receiver &&receiver) {
    ::p0443_v2::submit(std::forward<Sender>(sender), std::forward<Receiver>(receiver));
}
} // namespace tag
} // namespace p0443_v2