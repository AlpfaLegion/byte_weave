cmake_minimum_required(VERSION 3.15)
project(examples_byte_weave LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)

add_executable(
    ${PROJECT_NAME}

    advance_usage.cpp
    # basic_usage.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE byte_weave)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

