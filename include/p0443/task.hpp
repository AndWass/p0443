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
    task(std::function<R()> fun, Executor exec) : fun_(std::move(fun)), exec_(std::move(exec)) {
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
        execution::execute(exec_, [recv = std::move(rx), fun = std::move(fun_)]() mutable {
            try {
                fun();
                execution::set_value(std::move(recv));
            }
            catch (const std::exception &e) {
                execution::set_error(std::move(recv), e);
            }
        });
    }
    template <class Receiver>
    void tagged_submit(Receiver &&rx, std::false_type) {
        try {
            execution::set_value(std::forward<Receiver>(rx), fun_());
        }
        catch (const std::exception &e) {
            execution::set_error(std::forward<Receiver>(rx), e);
        }
    }
};
} // namespace p0443