cmake_minimum_required(VERSION 3.8)
project(p0443)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(p0443 INTERFACE)
target_compile_features(p0443 INTERFACE cxx_std_17)

include(GNUInstallDirs)

target_include_directories(p0443 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

install(
    TARGETS p0443
    EXPORT p0443-config
)
install(EXPORT p0443-config
    NAMESPACE p0443::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/p0443
)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(P0443_BUILD_TESTS)
    add_subdirectory(tests)
endif()

