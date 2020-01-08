#pragma once

#include <type_traits>

namespace p0443::util
{
template< class T >
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;

template<class T>
using is_nothrow_move_or_copy_constructible =
    std::disjunction<std::is_nothrow_move_constructible<T>, std::is_copy_constructible<T>>;
}