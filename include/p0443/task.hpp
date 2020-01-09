#pragma once

#include "execution/execution.hpp"

#include <functional>
#include <type_traits>

namespace p0443
{
template <class R, class Executor>
class task
{
public:
    using value_type = R;
    using error_type = std::exception;

    template <class Fn>
    task(Fn fun, Executor exec) : fun_(std::move(fun)), exec_(std::move(exec)) {
    }

    template <class Receiver>
    void submit(Receiver &&rx) {
        tagged_submit(std::forward<Receiver>(rx), std::is_void<R>{});
    }

private:
    std::function<R()> fun_;
    Executor exec_;

    template <class Receiver>
    void tagged_submit(Receiver &&rx, std::true_type) {
        execution::execute(exec_, [recv = std::move_if_noexcept(rx), fun = std::move_if_noexcept(fun_)]() mutable {
            try {
                fun();
                execution::set_value(recv);
            }
            catch (const std::exception &e) {
                execution::set_error(recv, e);
            }
        });
    }
    template <class Receiver>
    void tagged_submit(Receiver &&rx, std::false_type) {
        execution::execute(exec_, [recv = std::move_if_noexcept(rx), fun = std::move_if_noexcept(fun_)]() mutable {
            try {
                execution::set_value(recv, fun());
            }
            catch (const std::exception &e) {
                execution::set_error(recv, e);
            }
        });
    }
};

template <class Function, class Executor>
auto make_task(Function &&fn, Executor &&exec) {
    using invoke_result = std::invoke_result_t<Function>;
    return task<invoke_result, std::decay_t<Executor>>(std::forward<Function>(fn),
                                                       std::forward<Executor>(exec));
}

} // namespace p0443