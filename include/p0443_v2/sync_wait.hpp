#pragma once

#include <p0443_v2/submit.hpp>

#include <atomic>
#include <exception>
#include <optional>
#include <thread>

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

struct sync_wait_impl
{
    template <class R, class Sender>
    static auto wait(Sender &&sender) {
        shared_state<R> state;
        p0443_v2::submit(std::forward<Sender>(sender), state.ref());
        return state.get();
    }
};
} // namespace detail
using sync = detail::sync_wait_impl;
} // namespace p0443_v2