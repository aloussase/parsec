#include <iostream>

#include "parsec/adapter.hpp"
#include "parsec/numeric.hpp"
#include "parsec/parsec.hpp"

using namespace parsec;

struct Person
{
  std::string name;
  int age;

  Person(const std::string& n, const std::string& a)
  {
    name = n;
    age = std::stoi(a);
  }
};

[[nodiscard]] Person
mkPerson(const std::string& name, const std::string& age) noexcept
{
  return { name, age };
}

auto
main() -> int
{
  auto parser = curry2(construct<Person>{}) % (stringP("Alexander") < charP(' ')) * digitsP();
  // Or
  // auto parser = curry2(mkPerson) % (stringP("Alexander") < charP(' ')) * digitsP();
  auto person = parser.runThrowing("Alexander 23");
  std::cout << person.name << " " << person.age << "\n";
  return 0;
}
