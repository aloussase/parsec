add_executable(person_parser person_parser.cpp)
target_link_libraries(person_parser PUBLIC parsec)

add_executable(json_parser json.cpp)
target_link_libraries(json_parser PUBLIC parsec)
target_compile_options(json_parser PUBLIC -Wall -Wextra -g -fsanitize=address)
target_link_libraries(json_parser PUBLIC asan)
