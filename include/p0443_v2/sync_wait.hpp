
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/sender_traits.hpp>
#include <p0443_v2/connect.hpp>
#include <p0443_v2/start.hpp>

#include <atomic>
#include <exception>
#include <optional>
#include <thread>
#include <variant>
#include <tuple>

namespace p0443_v2
{
namespace detail
{
template <class T>
struct shared_state
{
    using value_type = std::decay_t<T>;
    class reference
    {
    public:
        reference(shared_state &state) : state_(& state) {
        }

        void set_value() {
            this->set_done();
        }

        template <class T2, class... Rest>
        std::enable_if_t<std::is_convertible_v<std::decay_t<T2>, value_type>>
        set_value(T2 &&v, Rest &&...) {
            state_->value = std::forward<T2>(v);
            state_->has_been_set_.store(true);
        }

        void set_done() {
            state_->value = std::nullopt;
            state_->has_been_set_.store(true);
        }

        void set_error(std::exception_ptr ex) {
            state_->exception_ = ex;
            state_->has_been_set_.store(true);
        }

    private:
        shared_state *state_;
    };

    T get() const {
        while (!has_been_set_.load()) {
            std::this_thread::yield();
        }
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        return *value;
    }

    reference ref() {
        return reference(*this);
    }

private:
    std::exception_ptr exception_;
    std::atomic_bool has_been_set_{false};
    std::optional<T> value;
};

template <>
struct shared_state<void>
{
    class reference
    {
    public:
        reference(shared_state &state) : state_(& state) {
        }

        template <class... Values>
        void set_value(Values &&... v) {
            state_->has_been_set_.store(true);
        }

        void set_done() {
            state_->has_been_set_.store(true);
        }

        void set_error(std::exception_ptr ex) {
            state_->exception_ = ex;
            state_->has_been_set_.store(true);
        }

    private:
        shared_state *state_;
    };

    reference ref() {
        return reference(*this);
    }

    void get() const {
        while (!has_been_set_.load()) {
            std::this_thread::yield();
        }
        if (exception_) {
            std::rethrow_exception(exception_);
        }
    }

private:
    std::exception_ptr exception_;
    std::atomic_bool has_been_set_{false};
};

template<class T>
struct valued_sync_wait_impl
{
    template<class Sender>
    static auto run(Sender &&sender)
    {
        using decayed = std::decay_t<Sender>;
        using value_types = typename p0443_v2::sender_traits<decayed>::template value_types<std::tuple, std::variant>;
        shared_state<value_types> state;
        auto op = p0443_v2::connect(std::forward<Sender>(sender), state.ref());
        p0443_v2::start(op);
        return state.get();
    }
};

template<class T>
struct valued_sync_wait_impl<std::variant<std::tuple<T>>>
{
    template<class Sender>
    static auto run(Sender &&sender)
    {
        shared_state<T> state;
        auto op = p0443_v2::connect(std::forward<Sender>(sender), state.ref());
        p0443_v2::start(op);
        return state.get();
    }
};

template<>
struct valued_sync_wait_impl<std::variant<std::tuple<>>>
{
    template<class Sender>
    static auto run(Sender &&sender)
    {
        shared_state<void> state;
        auto op = p0443_v2::connect(std::forward<Sender>(sender), state.ref());
        p0443_v2::start(op);
        state.get();
    }
};

struct sync_wait_cpo
{
    template <class Sender>
    constexpr auto operator()(Sender &&sender) const {
        using decayed = std::decay_t<Sender>;
        using value_types = typename p0443_v2::sender_traits<decayed>::template value_types<std::tuple, std::variant>;
        return p0443_v2::detail::valued_sync_wait_impl<value_types>::run(std::forward<Sender>(sender));
    }
};
} // namespace detail
detail::sync_wait_cpo sync_wait;
} // namespace p0443_v2