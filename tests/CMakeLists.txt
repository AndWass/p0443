
set(sources
    main.cpp
    execution.cpp
    task.cpp
    v2/tag_invoke.cpp
    v2/submit.cpp
    v2/transform.cpp
    v2/sync_wait.cpp
    v2/let.cpp
    v2/asio_executor.cpp
    v2/handle_error.cpp
    v2/asio_connect.cpp
    v2/just.cpp
    v2/sender_traits.cpp
    v2/sequence.cpp
)

if(P0443_BUILD_CORO_TESTS)
    list(APPEND sources
        v2/await_sender.cpp
    )
endif()

add_executable(p0443-tests
    ${sources}
)

target_link_libraries(p0443-tests PRIVATE p0443
    CONAN_PKG::doctest
    CONAN_PKG::boost
)
target_compile_definitions(p0443-tests PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )