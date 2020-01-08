#pragma once
#include "detail/execution.hpp"

namespace p0443
{
namespace execution
{
constexpr detail::execute_impl execute;
constexpr detail::submit_impl submit;
constexpr detail::set_value_impl set_value;
constexpr detail::set_error_impl set_error;
constexpr detail::set_done_impl set_done;
constexpr detail::schedule_impl schedule;
}
}