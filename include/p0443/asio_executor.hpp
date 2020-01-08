#pragma once

#include <utility>

namespace p0443
{
template<class Executor>
struct asio_executor
{
    explicit asio_executor(Executor &ex) {
        executor_ = std::addressof(ex);
    }

    template<class Fn>
    void execute(Fn &&fn) {
        executor_->post(std::forward<Fn>(fn));
    }

    Executor* executor_;
};
}