#pragma once

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