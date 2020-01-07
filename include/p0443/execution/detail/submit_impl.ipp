#pragma once

#include <exception>

#include "../../util/type_traits.hpp"
#include "execute_impl.hpp"
#include "submit_impl.hpp"

namespace p0443::execution::detail
{
template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_member, Tx &&tx, Rx &&rx) const {
    std::forward<Tx>(tx).submit(std::forward<Rx>(rx));
}

template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_free, Tx &&tx, Rx &&rx) const {
    submit(std::forward<Tx>(tx), std::forward<Rx>(rx));
}

template <typename R>
struct as_invocable
{
private:
    using receiver_type = util::remove_cvref_t<R>;
    std::optional<receiver_type> r_{};
    template<class T>
    void try_init_(T &&r)
    {
        try
        {
            r_.emplace((decltype(r) &&)r);
        }
        catch (...)
        {
            detail::set_error_impl{}(r, std::current_exception());
        }
    }

public:
    explicit as_invocable(receiver_type &&r)
    {
        try_init_(std::move_if_noexcept(r));
    }
    explicit as_invocable(const receiver_type &r)
    {
        try_init_(r);
    }
    as_invocable(as_invocable && other)
    {
        if (other.r_)
        {
            try_init_(move_if_noexcept(*other.r_));
            other.r_.reset();
        }
    }
    ~as_invocable()
    {
        if (r_)
            detail::set_done_impl{}(*r_);
    }
    void operator()()
    {
        try
        {
            detail::set_value_impl{}(*r_);
        }
        catch (...)
        {
            detail::set_error_impl{}(*r_, std::current_exception());
        }
        r_.reset();
    }
};

template <typename Tx, typename Rx>
void submit_impl::tagged_submit(tag_indirection, Tx &&tx, Rx &&rx) const {
    execute_impl{}(std::forward<Tx>(tx), as_invocable<Rx>(std::forward<Rx>(rx)));
}
} // namespace p0443::execution::detail