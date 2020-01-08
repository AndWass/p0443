add_executable(p0443-tests
    main.cpp
    execution.cpp
    task.cpp
)

target_link_libraries(p0443-tests PRIVATE p0443
    CONAN_PKG::doctest
    CONAN_PKG::boost
)
target_compile_definitions(p0443-tests PRIVATE
    _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    _WIN32_WINNT=0x0601
    )
