cmake_minimum_required(VERSION 3.15)

project(byte_weave VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BYTE_WEAVE_BUILD_TESTS "Build tests" ON)
option(BYTE_WEAVE_BUILD_EXAMPLES "Build examples" OFF)
option(BYTE_WEAVE_INSTALL_SINGLE_HEADER "Install generated single-header instead of multi-header include tree" OFF)

add_library(byte_weave INTERFACE)
add_library(byte_weave::byte_weave ALIAS byte_weave)

target_compile_features(byte_weave INTERFACE cxx_std_23)

if(BYTE_WEAVE_INSTALL_SINGLE_HEADER)
    target_include_directories(byte_weave INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/single_include>
        $<INSTALL_INTERFACE:include>
    )
else()
    target_include_directories(byte_weave INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

if(BYTE_WEAVE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BYTE_WEAVE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
    TARGETS byte_weave
    EXPORT byte_weaveTargets
)

if(BYTE_WEAVE_INSTALL_SINGLE_HEADER)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/single_include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
else()
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

install(
    EXPORT byte_weaveTargets
    FILE byte_weaveTargets.cmake
    NAMESPACE byte_weave::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/byte_weave
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/byte_weaveConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/byte_weaveConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/byte_weave
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/byte_weaveConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/byte_weaveConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/byte_weaveConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/byte_weave
)
