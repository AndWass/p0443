function(set_exec_props target)
    target_link_libraries(${target} PRIVATE
        p0443
        CONAN_PKG::boost
        )

    target_compile_definitions(${target} PRIVATE
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
        _WIN32_WINNT=0x0601
        )
endfunction(set_exec_props)

add_executable(samples-asio-tcp-echo-server tcp_echo_server.cpp)
add_executable(samples-asio-tcp-echo-server-stack-only tcp_echo_server_stack_only.cpp)
add_executable(samples-asio-tcp-echo-client tcp_echo_client.cpp)

set_exec_props(samples-asio-tcp-echo-server)
set_exec_props(samples-asio-tcp-echo-server-stack-only)
set_exec_props(samples-asio-tcp-echo-client)

if(P0443_BUILD_CORO_SAMPLES)
    add_executable(samples-coro-asio-tcp-echo-server tcp_coro_echo_server.cpp)
    add_executable(samples-coro-asio-tcp-echo-client tcp_coro_echo_client.cpp)

    set_exec_props(samples-coro-asio-tcp-echo-server)
    set_exec_props(samples-coro-asio-tcp-echo-client)
endif()