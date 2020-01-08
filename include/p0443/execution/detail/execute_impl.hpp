#pragma once

#include "../../util/type_traits.hpp"
#include "../../util/detect.hpp"
#include "tag.hpp"

namespace p0443::execution::detail
{
void execute();
struct execute_impl
{
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

    template <class Ex, class Fn, class NoRef = std::remove_reference_t<Ex>>
    using member_execute_t = decltype(std::declval<NoRef>().execute(std::declval<Fn>()));

    template <class Ex, class Fn>
    using free_execute_t = decltype(execute(std::declval<Ex>(), std::declval<Fn>()));

    template<class Ex, class Fn>
    using use_member_t = util::is_detected<member_execute_t, Ex, Fn>;

    template <class Ex, class Fn>
    static constexpr bool use_member = use_member_t<Ex, Fn>::value;

    template<class Ex, class Fn>
    using use_free_t = util::is_detected<free_execute_t, Ex, Fn>;

    template <class Ex, class Fn>
    static constexpr bool use_free = use_free_t<Ex, Fn>::value;

    template<class Ex, class Fn>
    using use_indirection_t = std::conjunction<
        std::negation<use_member_t<Ex, Fn>>,
        std::negation<use_free_t<Ex, Fn>>,
        std::is_invocable<Fn>
    >;

    template <class Ex, class Fn>
    static constexpr bool use_indirection = use_indirection_t<Ex, Fn>::value;

    template <typename Ex, typename Fn, std::enable_if_t<
        std::disjunction<
        use_member_t<Ex, Fn>,
        use_free_t<Ex, Fn>,
        use_indirection_t<Ex, Fn>
        >::value
    >* = nullptr>
    void operator()(Ex && ex, Fn && fn) const {
        this->tagged_execute(std::forward<Ex>(ex), std::forward<Fn>(fn),
                             execution_tag_t<execute_impl, Ex, Fn>{});
    }

private:
    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_member) const;

    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_free) const;

    template <typename Ex, typename Fn>
    void tagged_execute(Ex &&ex, Fn &&fn, tag_indirection) const;
};
} // namespace p0443::execution::detail