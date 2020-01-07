#pragma once

#include "../../util/utility.hpp"
#include "../../util/type_traits.hpp"
#include "execute_impl.hpp"

namespace p0443::execution::detail
{
template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_member) const {
    ex.execute(util::decay_copy(std::forward<Fn>(fn)));
}

template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_free) const {
    execute(std::forward<Ex>(ex), util::decay_copy(std::forward<Fn>(fn)));
}

template <typename F>
struct as_receiver
{
private:
    using invocable_type = util::remove_cvref_t<F>;
    invocable_type f_;

public:
    explicit as_receiver(invocable_type &&f) : f_(std::move_if_noexcept(f)) {
    }
    explicit as_receiver(const invocable_type &f) : f_(f) {
    }
    as_receiver(as_receiver &&other) = default;
    void set_value() {
        std::invoke(f_);
    }
    void set_error(std::exception_ptr) {
        std::terminate();
    }
    void set_done() noexcept {
    }
};

template <typename Ex, typename Fn>
void execute_impl::tagged_execute(Ex &&ex, Fn &&fn, tag_indirection) const {
    submit_impl{}(std::forward<Ex>(ex), as_receiver<Fn>{std::forward<Fn>(fn)});
}
} // namespace p0443::execution::detail