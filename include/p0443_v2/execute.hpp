#pragma once

#include "tag_invoke.hpp"
#include "type_traits.hpp"

#include <exception>
#include <optional>
#include <utility>
#include <functional>

namespace p0443_v2
{
namespace detail
{
template <class Executor, class Function>
using has_execute_member_detector =
    decltype(std::declval<Executor>().execute(std::declval<Function>()));

void execute();
template <class Executor, class Function>
using has_execute_free_detector =
    decltype(execute(std::declval<Executor>(), std::declval<Function>()));

template <class Function>
struct as_receiver
{
private:
    using invocable_type = p0443_v2::remove_cvref_t<Function>;
    invocable_type f_;

public:
    explicit as_receiver(invocable_type &&f) : f_(std::move_if_noexcept(f)) {
    }
    explicit as_receiver(const invocable_type &f) : f_(f) {
    }
    as_receiver(as_receiver && other) = default;

    void set_value() {
        std::invoke(f_);
    }
    void set_error(std::exception_ptr) {
        std::terminate();
    }
    void set_done() noexcept {
    }
};

struct execute_impl
{
    template<class Executor, class Function>
    using use_member = p0443_v2::is_detected<detail::has_execute_member_detector, Executor, Function>;

    template<class Executor, class Function>
    using use_free_function = std::conjunction<
        std::negation<use_member<Executor, Function>>,
        p0443_v2::is_detected<detail::has_execute_free_detector, Executor, Function>
    >;

    template<class Executor, class Function>
    using use_submit = std::conjunction<
        std::negation<use_member<Executor, Function>>,
        std::negation<use_free_function<Executor, Function>>,
        std::is_invocable<Function>
    >;

    template <class Executor, class Function>
    std::enable_if_t<use_member<Executor, Function>::value>
    operator()(Executor && executor, Function && function) const {
        executor.execute(std::forward<Function>(function));
    }

    template <class Executor, class Function>
    std::enable_if_t<use_free_function<Executor, Function>::value>
    operator()(Executor && executor, Function && function) const {
        execute(std::forward<Executor>(executor), std::forward<Function>(function));
    }

    template <class Executor, class Function>
    std::enable_if_t<use_submit<Executor, Function>::value>
    operator()(Executor && executor, Function && function) const {
        ::p0443_v2::tag_invoke(std::forward<Executor>(executor), ::p0443_v2::tag::submit,
                               as_receiver(std::forward<Function>(function)));
    }
};
} // namespace detail
constexpr detail::execute_impl execute;

namespace tag
{
template <class Executor, class Function>
void tag_invoke(Executor &&executor, p0443_v2::tag::execute_t, Function &&function) {
    ::p0443_v2::execute(std::forward<Executor>(executor), std::forward<Function>(function));
}

} // namespace tag
} // namespace p0443_v2