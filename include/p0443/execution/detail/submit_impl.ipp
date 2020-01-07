#pragma once

#include "execute_impl.hpp"
#include "submit_impl.hpp"
#include "../../util/type_traits.hpp"

namespace p0443::execution::detail
{
template <class Tx, class Rx>
std::enable_if_t<submit_impl::use_member<Tx, Rx>> submit_impl::operator()(Tx && tx, Rx && rx) const
{
    std::forward<Tx>(tx).submit(std::forward<Rx>(rx));
}

template <class Tx, class Rx>
std::enable_if_t<submit_impl::use_free<Tx, Rx>> submit_impl::operator()(Tx && tx, Rx && rx) const
{
    submit(std::forward<Tx>(tx), std::forward<Rx>(rx));
}

/*template <typename R>
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
            execution::set_error(r, current_exception());
        }
    }

public:
    explicit as_invocable(receiver_type &&r)
    {
        try_init_(move_if_noexcept(r));
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
            execution::set_done(*r_);
    }
    void operator()()
    {
        try
        {
            execution::set_value(*r_);
        }
        catch (...)
        {
            execution::set_error(*r_, current_exception());
        }
        r_.reset();
    }
};

template <class Tx, class Rx>
std::enable_if_t<submit_impl::use_execute<Tx, Rx>> submit_impl::operator()(Tx && tx, Rx && rx) const
{
    execute_impl{}(std::forward<Tx>(tx), as_invocable<Rx>(std::forward<Rx>(rx)));
}*/
}