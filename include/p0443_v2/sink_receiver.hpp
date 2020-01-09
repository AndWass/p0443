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