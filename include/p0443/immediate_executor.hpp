#pragma once

#include <p0443_v2/execute.hpp>

#include <functional>

namespace p0443
{
struct immediate_executor
{
    template <typename Fn>
    void execute(Fn &&fn) const {
        fn();
    }
};
} // namespace p0443