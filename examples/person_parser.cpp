#include <iostream>

#include "parsec/adapter.hpp"
#include "parsec/parsec.hpp"
#include "parsec/parsers.hpp"

using namespace parsec;

struct Person : public all_args_factory<Person>
{
  const std::string name;
  const int age;
};

auto
main() -> int
{
  auto wordP = many1(letter()) & convert::tostring();
  auto parser = make<Person, std::string, int>() % (wordP < charP(' '))
              * decimal();
  auto person = parser.runThrowing("Alexander 23");
  std::cout << person.name << " " << person.age << "\n";
  return 0;
}
