#pragma once

#include "../../util/detect.hpp"
#include "../../util/type_traits.hpp"

#include <type_traits>

#include "../executor_traits.hpp"
#include "../receiver_traits.hpp"

namespace p0443::execution::detail
{
void submit();
struct submit_impl
{
    template <typename R>
    struct as_invocable
    {
    private:
        using receiver_type = util::remove_cvref_t<R>;
        std::optional<receiver_type> r_{};
        template <class T>
        void try_init_(T &&r) {
            try {
                r_.emplace((decltype(r) &&)r);
            }
            catch (...) {
                detail::set_error_impl{}(r, std::current_exception());
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
                detail::set_done_impl{}(*r_);
        }
        void operator()() {
            try {
                detail::set_value_impl{}(*r_);
            }
            catch (...) {
                detail::set_error_impl{}(*r_, std::current_exception());
            }
            r_.reset();
        }
    };

    template <typename Tx, typename Rx, typename NoRef = std::remove_reference_t<Tx>>
    using member_submit_t = decltype(std::declval<NoRef>().submit(std::declval<Rx>()));

    template <typename Tx, typename Rx>
    using free_submit_t = decltype(submit(std::declval<Tx>(), std::declval<Rx>()));

    template <typename Tx, typename Rx>
    using use_member_t = util::is_detected<member_submit_t, Tx, Rx>;

    template <typename Tx, typename Rx>
    static constexpr bool use_member = use_member_t<Tx, Rx>::value;

    template <typename Tx, typename Rx>
    using use_free_t = std::conjunction<std::negation<use_member_t<Tx, Rx>>,
                                        util::is_detected<free_submit_t, Tx, Rx>>;

    template <typename Tx, typename Rx>
    static constexpr bool use_free = use_free_t<Tx, Rx>::value;

    template <typename Tx, typename Rx>
    static constexpr bool use_indirection =
        std::conjunction<std::negation<use_member_t<Tx, Rx>>, std::negation<use_free_t<Tx, Rx>>,
                         is_receiver_of<Rx>, is_executor<Tx>>::value;

    template <typename Tx, typename Rx,
              std::enable_if_t<use_member<Tx, Rx> || use_free<Tx, Rx> || use_indirection<Tx, Rx>>
                  * = nullptr>
    void operator()(Tx && tx, Rx && rx) const {
        tagged_submit(execution_tag_t<submit_impl, Tx, Rx>{}, std::forward<Tx>(tx),
                      std::forward<Rx>(rx));
    }

private:
    template <typename Tx, typename Rx>
    void tagged_submit(tag_member, Tx &&tx, Rx &&rx) const;

    template <typename Tx, typename Rx>
    void tagged_submit(tag_free, Tx &&tx, Rx &&rx) const;

    template <typename Tx, typename Rx>
    void tagged_submit(tag_indirection, Tx &&tx, Rx &&rx) const;
};
} // namespace p0443::execution::detail