add_executable(samples-asio-tcp-echo tcp_echo.cpp)
target_link_libraries(samples-asio-tcp-echo PRIVATE
    p0443
    CONAN_PKG::boost
    )

target_compile_definitions(samples-asio-tcp-echo PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )