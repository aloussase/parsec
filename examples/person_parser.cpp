#include <iostream>

#include "parsec/adapter.hpp"
#include "parsec/parsec.hpp"
#include "parsec/parsers.hpp"

using namespace parsec;

struct Person
{
  std::string name;
  int age;

  Person(const std::string& n, int a) : name{ n }, age{ a } {}
};

[[nodiscard]] Person
mkPerson(const std::string& name, int age) noexcept
{
  return { name, age };
}

auto
main() -> int
{
  auto wordP = many1(letter()) & convert::tostring();
  auto parser = make<Person, 2>() % (wordP < charP(' ')) * decimal();
  // Or
  // auto parser = curry<2>(mkPerson) % (stringP("Alexander") < charP(' ')) * digitsP();
  auto person = parser.runThrowing("Alexander 23");
  std::cout << person.name << " " << person.age << "\n";
  return 0;
}
