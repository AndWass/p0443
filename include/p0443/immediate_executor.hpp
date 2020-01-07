#pragma once

#include <functional>

namespace p0443
{
struct immediate_executor
{
    template<typename Fn>
    void execute(Fn && fn) {
        std::invoke(std::forward<Fn>(fn));
    }
};
}