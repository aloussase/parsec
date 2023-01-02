# parsec

A parser combinator library for C++ inspired on
[attoparsec](https://hackage.haskell.org/package/attoparsec-0.14.4/docs/Data-Attoparsec-Text.html).

## Example

See the `examples` folder for more.

```cpp
#include <iostream>

#include "parsec/all.hpp"

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
}```

## Usage

The code is well commented. Also, the library tries to follow the style of
[attoparsec](https://hackage.haskell.org/package/attoparsec-0.14.4/docs/Data-Attoparsec-Text.html),
so you can read that too.

The following is a list of the operators provided by the library:

| Operator  | Description                                                   |
| --------- | --------------------------------------------------------------|
| `%`       | Functorial application (fmap)                                 |
| `&`       | % with its arguments swapped                                  |
| `*`       | Applicative application (<*>)                                 |
| `>>=`     | Monadic bind                                                  |
| `\|`      | Alternative operation                                         |
| `>>`      | Bind, discarding the result of the first computation          |
| `>`       | Same as `>>`                                                  |
| `<`       | Run two computations and return the result of the first one   |


## TODO

- [x] Labels for parsers
- [ ] Track position in input
- [ ] Benchmarks
- [ ] Documentation

## Resources

- [Build your own parser combinator
  library](https://fsharpforfunandprofit.com/posts/understanding-parser-combinators-3/)

