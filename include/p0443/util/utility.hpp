#pragma once

#include <type_traits>

namespace p0443::util
{
template<class T>
typename std::decay<T>::type decay_copy(T&& t) {
    return std::forward<T>(t);
}
}