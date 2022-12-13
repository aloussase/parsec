include(CTest)

add_executable(parsec_test parsec_test.cpp)
target_link_libraries(parsec_test PUBLIC parsec)
add_test(NAME ParsecTest COMMAND parsec_test)
