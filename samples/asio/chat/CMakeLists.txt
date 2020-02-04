add_executable(samples-asio-chat-server chat-server.cpp)
target_link_libraries(samples-asio-chat-server PRIVATE
    p0443
    CONAN_PKG::boost
    )

target_compile_definitions(samples-asio-chat-server PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )


add_executable(samples-asio-chat-client chat-client.cpp)
target_link_libraries(samples-asio-chat-client PRIVATE
    p0443
    CONAN_PKG::boost
    )

target_compile_definitions(samples-asio-chat-client PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )
    