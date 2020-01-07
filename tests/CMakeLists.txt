add_executable(p0443-tests
    main.cpp
    execution.cpp
    task.cpp
)

target_link_libraries(p0443-tests PRIVATE p0443
    CONAN_PKG::doctest
    CONAN_PKG::boost
)
