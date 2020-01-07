#pragma once

namespace p0443::execution::detail
{
struct tag_member
{
};
struct tag_free
{
};
struct tag_indirection
{
};

template <bool A, bool, bool>
struct execution_tag
{ static_assert(!A, "Invalid tag type"); };

template <bool B, bool C>
struct execution_tag<true, B, C>
{ using type = tag_member; };

template <bool C>
struct execution_tag<false, true, C>
{ using type = tag_free; };

template <>
struct execution_tag<false, false, true>
{ using type = tag_indirection; };

template <class T, typename... Args>
using execution_tag_t =
    typename execution_tag<T::template use_member<Args...>, T::template use_free<Args...>,
                           T::template use_indirection<Args...>>::type;

} // namespace p0443::execution::detail