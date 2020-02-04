
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "set_done.hpp"
#include "set_error.hpp"
#include "set_value.hpp"

namespace p0443_v2
{

class sink_receiver
{
public:
    template <class... Values>
    void set_value(Values &&...) {
    }
    template <class Error>
    [[noreturn]] void set_error(Error &&) noexcept {
        std::terminate();
    }
    void set_done() noexcept {
    }
};

} // namespace p0443_v2