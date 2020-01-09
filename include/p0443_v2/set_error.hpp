#pragma once

#include "tag_invoke.hpp"
#include "type_traits.hpp"
#include <type_traits>

namespace p0443_v2
{
namespace detail
{
void set_error();
struct set_error_impl
{
    template <class Receiver, class Error>
    using member_detector = decltype(std::declval<Receiver>().set_error(std::declval<Error>()));

    template <class Receiver, class Error>
    using free_fn_detector = decltype(set_error(std::declval<Receiver>(), std::declval<Error>()));

    template <class Receiver, class Error>
    using use_member = p0443_v2::is_detected<member_detector, Receiver, Error>;

    template <class Receiver, class Error>
    using use_free_function =
        std::conjunction<std::negation<use_member<Receiver, Error>>,
                         p0443_v2::is_detected<free_fn_detector, Receiver, Error>>;

    template <class Receiver, class Error>
    std::enable_if_t<use_member<Receiver, Error>::value> operator()(Receiver && rx,
                                                                    Error && err) const noexcept {
        rx.set_error(std::forward<Error>(err));
    }

    template <class Receiver, class Error>
    std::enable_if_t<use_free_function<Receiver, Error>::value> operator()(Receiver && rx,
                                                                           Error && err) const noexcept {
        set_error(std::forward<Receiver>(rx), std::forward<Error>(err));
    }
};
} // namespace detail
constexpr detail::set_error_impl set_error;

namespace tag
{
template <class Receiver, class Error>
std::enable_if_t<std::is_invocable_v<::p0443_v2::detail::set_error_impl, Receiver, Error>>
tag_invoke(Receiver &&rx, ::p0443_v2::tag::set_error_t, Error &&err) {
    ::p0443_v2::set_error(std::forward<Receiver>(rx), std::forward<Error>(err));
}
} // namespace tag

} // namespace p0443_v2