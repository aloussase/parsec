include(CTest)

add_executable(parsec_test parsec_test.cpp)
target_compile_options(parsec_test PUBLIC -g -fsanitize=address)
target_link_libraries(parsec_test PUBLIC parsec asan)
add_test(NAME ParsecTest COMMAND parsec_test)
