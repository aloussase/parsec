cmake_minimum_required(VERSION 3.9)
cmake_policy(VERSION 3.9)

project(parsec LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_library(parsec INTERFACE)
target_include_directories(parsec INTERFACE .)
target_compile_options(parsec INTERFACE -Wall -Wextra)

add_executable(parsec_test parsec_test.cpp)
target_link_libraries(parsec_test PUBLIC parsec)
add_test(NAME ParsecTest COMMAND parsec_test)
