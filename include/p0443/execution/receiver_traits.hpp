#pragma once

#include "execution.hpp"
#include "p0443/util/type_traits.hpp"
#include "detail/set_done_impl.hpp"
#include "detail/set_value_impl.hpp"
#include "detail/set_error_impl.hpp"

#include <exception>

namespace p0443::execution
{
template<class T, class E = std::exception_ptr>
using is_receiver = std::conjunction<
    std::is_move_constructible<util::remove_cvref_t<T>>,
    util::is_nothrow_move_or_copy_constructible<util::remove_cvref_t<T>>,
    std::is_invocable<execution::detail::set_done_impl, T&&>,
    std::is_invocable<execution::detail::set_error_impl, T&&, E&&>
    >;

template<class T, class E = std::exception_ptr>
constexpr bool is_receiver_v = is_receiver<T, E>::value;

template<class T, class...Args>
using is_receiver_of = std::conjunction<
        is_receiver<T>,
        std::is_invocable<execution::detail::set_value_impl, T&&, Args&&...>
    >;

template<class T, class...Args>
constexpr bool is_receiver_of_v = is_receiver_of<T, Args...>::value;

}