#pragma once

#include "execute_impl.hpp"
#include "../../util/type_traits.hpp"

namespace p0443::execution::detail
{
template <typename Ex, typename Fn>
std::enable_if_t<execute_impl::use_member<Ex, Fn>> execute_impl::operator()(Ex && ex, Fn && fn) const
{
    std::forward<Ex>(ex).execute(std::forward<Fn>(fn));
}

template <typename Ex, typename Fn>
std::enable_if_t<execute_impl::use_free<Ex, Fn>> execute_impl::operator()(Ex && ex, Fn && fn) const
{
    execute(std::forward<Ex>(ex), std::forward<Fn>(fn));
}

template <typename F>
struct as_receiver
{
private:
    using invocable_type = util::remove_cvref_t<F>;
    invocable_type f_;

public:
    explicit as_receiver(invocable_type &&f) : f_(std::move_if_noexcept(f)) {}
    explicit as_receiver(const invocable_type &f) : f_(f) {}
    as_receiver(as_receiver &&other) = default;
    void set_value()
    {
        std::invoke(f_);
    }
    void set_error(std::exception_ptr)
    {
        std::terminate();
    }
    void set_done() noexcept {}
};

template <typename Ex, typename Fn>
std::enable_if_t<execute_impl::use_submit<Ex, Fn>> execute_impl::operator()(Ex && ex, Fn && fn) const
{
    submit_impl{}(std::forward<Ex>(ex), as_receiver<Fn>{std::forward<Fn>(fn)});
}
}