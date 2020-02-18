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

add_executable(samples-asio-chat-server chat-server.cpp)
add_executable(samples-asio-chat-client chat-client.cpp)

set_exec_props(samples-asio-chat-server)
set_exec_props(samples-asio-chat-client)

if(P0443_BUILD_CORO_SAMPLES)
    add_executable(samples-coro-asio-chat-client coro-chat-client.cpp)
    set_exec_props(samples-coro-asio-chat-client)
endif()