# parsec

A parser combinator library for C++ inspired on
[attoparsec](https://hackage.haskell.org/package/attoparsec-0.14.4/docs/Data-Attoparsec-Text.html).

## Example

See the `examples` folder for more.

```cpp
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

auto
main() -> int
{
  // Make a word parser from provided primitives.
  auto wordP = many1(letterP()) & convert::tostring();
  // Create the person parser.
  auto parser = curry<2>(make<Person>{}) % (wordP < charP(' ')) * decimalP();
  // Parse a person, throwing an exception if it fails.
  auto person = parser.runThrowing("Alexander 23");

  std::cout << person.name << " " << person.age << "\n";

  return 0;
}
```

## Usage

The code is well commented. Also, the library tries to follow the style of
[attoparsec](https://hackage.haskell.org/package/attoparsec-0.14.4/docs/Data-Attoparsec-Text.html),
so you can read that too.

The following is a list of the operators provided by the library:

| `%`   | Functorial application (fmap)                                 |
| `&`   | % with its arguments swapped                                  |
| `*`   | Applicative application (<*>)                                 |
| `>>=` | Monadic bind                                                  |
| `|`   | Alternative operation                                         |
| `>>`  | Bind, discarding the result of the first computation          |
| `>`   | Same as `>>`                                                  |
| `<`   | Run two computations and return the result of the first one   |


## TODO

- [ ] Labels for parsers
- [ ] Track position in input
- [ ] Benchmarks

## Resources

- [Build your own parser combinator
  library](https://fsharpforfunandprofit.com/posts/understanding-parser-combinators-3/)

