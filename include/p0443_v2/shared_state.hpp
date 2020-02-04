
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <memory>
#include <variant>

namespace p0443_v2
{
template<class T>
class shared_state
{
public:
    using element_type = std::decay_t<T>;
    using pointer = value_type*;
    using shared_pointer = std::shared_ptr<value_type>;

    shared_state(): value_(pointer(nullptr)) {}
    explicit shared_state(pointer p): value_(p) {}
    explicit shared_state(std::shared_ptr<value_type> ptr): value_(std::move(ptr)) {}

    pointer get() const {
        if(value_.index() == 0) {
            return std::get<0>(value_);
        }
        return std::get<1>(value_).get();
    }
    void reset() {
        value_ = pointer(nullptr);
    }
    pointer operator*() const {
        return get();
    }

    pointer operator->() const {
        return get();
    }
private:
    std::variant<pointer, shared_pointer> value_;
};
}