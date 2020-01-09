#pragma once

#include "tag_invoke.hpp"
#include "type_traits.hpp"
#include <type_traits>

namespace p0443_v2
{
namespace detail
{
void set_value();
struct set_value_impl
{
    template <class Receiver, class... Values>
    using member_detector = decltype(std::declval<Receiver>().set_value(std::declval<Values>()...));

    template <class Receiver, class... Values>
    using free_fn_detector =
        decltype(set_value(std::declval<Receiver>(), std::declval<Values>()...));

    template <class Receiver, class... Values>
    using use_member = p0443_v2::is_detected<member_detector, Receiver, Values...>;

    template <class Receiver, class... Values>
    using use_free_function =
        std::conjunction<std::negation<use_member<Receiver, Values...>>,
                         p0443_v2::is_detected<free_fn_detector, Receiver, Values...>>;

    template <class Receiver, class... Values>
    std::enable_if_t<use_member<Receiver, Values...>::value> operator()(Receiver && rx, Values &&
                                                                        ... values) const noexcept {
        rx.set_value(std::forward<Values>(values)...);
    }

    template <class Receiver, class... Values>
    std::enable_if_t<use_free_function<Receiver, Values...>::value>
    operator()(Receiver && rx, Values &&... values) const noexcept {
        set_value(std::forward<Receiver>(rx), std::forward<Values>(values)...);
    }
};
} // namespace detail
constexpr detail::set_value_impl set_value;

template<class Receiver, class...Values>
struct is_receiver_for_values: std::is_invocable<::p0443_v2::detail::set_value_impl, Receiver, Values...> {};

template<class Receiver, class...Values>
constexpr bool is_receiver_for_values_v = is_receiver_for_values<Receiver, Values...>::value;

namespace tag
{
template <class Receiver, class... Values>
std::enable_if_t<p0443_v2::is_receiver_for_values_v<Receiver, Values...>>
tag_invoke(Receiver &&rx, ::p0443_v2::tag::set_value_t, Values &&... err) {
    ::p0443_v2::set_value(std::forward<Receiver>(rx), std::forward<Values>(values)...);
}
} // namespace tag

} // namespace p0443_v2