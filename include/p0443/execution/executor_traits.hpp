#pragma once

#include "detail/execute_impl.hpp"
#include "p0443/util/type_traits.hpp"

#include <type_traits>
#include <exception>

namespace p0443::execution
{
namespace detail
{
struct invocable_archetype {
    void operator()();
};
}
template<class Executor, class Function>
using is_executor_of = std::conjunction<
    std::is_invocable<Function>,
    std::is_nothrow_copy_constructible<Executor>,
    std::is_nothrow_destructible<Executor>,
    std::is_invocable<execution::detail::execute_impl, const Executor&, Function&&>
    >;

template<class Executor, class Function>
constexpr bool is_executor_of_v = is_executor_of<Executor, Function>::value;

template<class Executor>
using is_executor = is_executor_of<Executor, detail::invocable_archetype>;

template<class Executor>
constexpr bool is_executor_v = is_executor<Executor>::value;

}