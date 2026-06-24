cmake_minimum_required(VERSION 3.15)

project(tests_byte_weave LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)

find_package(Catch2 3 REQUIRED)

add_executable(
    ${PROJECT_NAME}

    test_traits.cpp
    test_layouts.cpp
    test_message_schemas.cpp
    test_message_view.cpp
    test_message_span.cpp
    test_nativeblobs.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE byte_weave Catch2::Catch2WithMain)

target_include_directories(${PROJECT_NAME} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

include(Catch)

catch_discover_tests(
    tests_byte_weave
)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_CXX_COMPILER /usr/bin/g++)
# set(CMAKE_C_COMPILER /usr/bin/gcc)
# set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Catch2 3 REQUIRED)

# set(SOURCE 
#     # src/tests/test_layouts.cpp
#     # tests/test_message_schemas.cpp
#     test_message_view.cpp
#     )

# add_executable(${PROJECT_NAME} ${SOURCE})

# #target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
# #target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
# target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

# target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
# target_include_directories(
#     ${PROJECT_NAME}
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}
# )
