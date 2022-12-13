cmake_minimum_required(VERSION 3.9)
cmake_policy(VERSION 3.9)

project(parsec
        LANGUAGES CXX
        DESCRIPTION "A parser combinator library for C++ inspired by attoparsec.")

set(CMAKE_CXX_STANDARD 20)

add_library(parsec INTERFACE)
target_include_directories(
        parsec
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_options(parsec INTERFACE -Wall -Wextra)
target_compile_features(parsec INTERFACE cxx_std_20)

add_subdirectory(test)

