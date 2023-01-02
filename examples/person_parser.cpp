#include <iostream>

#include "parsec/all.hpp"

using namespace parsec;

struct Person : public all_args_factory<Person, std::string, int>
{
  const std::string name;
  const int age;
};

auto
main() -> int
{
  auto wordP = many1(letter()) & convert::tostring();
  auto parser = curry2(&Person::init) % (wordP < charP(' ')) * decimal();
  auto person = parser.runThrowing("Alexander 23");
  std::cout << person.name << " " << person.age << "\n";
  return 0;
}
