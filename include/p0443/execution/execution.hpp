#include <functional>

#include "detail/execution.hpp"

namespace p0443
{
namespace execution
{
constexpr detail::execute_impl execute;
constexpr detail::submit_impl submit;
//constexpr detail::set_value_impl set_value;
}
}