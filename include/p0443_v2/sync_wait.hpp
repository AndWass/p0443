#pragma once

#include <p0443_v2/submit.hpp>

#include <optional>
#include <condition_variable>
#include <mutex>
#include <exception>

namespace p0443_v2
{
namespace detail
{
template<class ValueT>
struct sync_wait_impl
{
    template<class T>
    struct shared_state
    {
        T get() {

        }
    };
    struct receiver
    {
        shared_state<ValueT> state;

        template<class...Values>
        void set_value(Values&&...vals) {
            state.value(std::forward<Values>(vals)...);
        }

        void set_done() {
            state.done();
        }

        void set_error(std::exception_ptr exception) {
            state.error(exception);
        }
    };
};
}
}