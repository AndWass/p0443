add_executable(samples-asio-tcp-echo-server tcp_echo_server.cpp)
target_link_libraries(samples-asio-tcp-echo-server PRIVATE
    p0443
    CONAN_PKG::boost
    )

target_compile_definitions(samples-asio-tcp-echo-server PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )


add_executable(samples-asio-tcp-echo-client tcp_echo_client.cpp)
target_link_libraries(samples-asio-tcp-echo-client PRIVATE
    p0443
    CONAN_PKG::boost
    )

target_compile_definitions(samples-asio-tcp-echo-client PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )