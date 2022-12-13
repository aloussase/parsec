//
// Created by aloussase on 12/12/22.
//

#include "parsec.hpp"

#include <cassert>

using namespace aoc::parser;

void
testSequenceCanParsePartOfItsInput()
{
  // Arrange
  auto charParser = charP('a') >> charP('o');

  // Act
  auto result = charParser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'o');
  assert(result.value().second == "c");
}

void
testCharPFailsToParseANonMatchingString()
{
  // Arrange
  auto failedParser = charP('c');

  // Act
  auto result = failedParser.run("aoc");

  // Assert
  assert(result.isFailure());
}

void
testCharPCanParseASingleCharacter()
{
  // Arrange
  auto charParser = charP('a');

  // Act
  auto result = charParser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'a');
}

void
testOrCanParseWhenGivenValidAlternatives()
{
  // Arrange
  auto parser = charP('a') | charP('b');

  // Act
  auto result = parser.run("aoc");
  auto result2 = parser.run("beef");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'a');
  assert(result.value().second == "oc");

  assert(result2.isSuccess());
  assert(result2.value().first == 'b');
  assert(result2.value().second == "eef");
}

void
testChoiceCanParseWhenGivenValidAlternatives()
{
  // Arrange
  auto parser = choice({
      charP('a'),
      charP('o'),
      charP('o'),
  });

  // Act
  auto result = parser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'a');
  assert(result.value().second == "oc");
}

void
testChoiceFailsWhenNoneOfItsParsersCanParseTheInput()
{
  // Arrange
  auto parser = choice({
      charP('a'),
      charP('o'),
      charP('o'),
  });

  // Act
  auto result = parser.run("2022");

  // Assert
  assert(result.isFailure());

  try
    {
      (void)result.value();
    }
  catch (ParserError& err)
    {
      assert(err.msg().find("choice") != std::string::npos);
    }
}

void
testAnyOfCanParseWhenGivenValidAlternatives()
{
  // Arrange
  auto parser = anyOf({ 'a', 'o', 'o' });

  // Act
  auto result = parser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'a');
  assert(result.value().second == "oc");
}

void
testMappingAParserWorks()
{
  // Arrange
  auto parser = charP('a').map([](char c) {
    return toupper(c);
  });

  // Act
  auto result = parser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'A');
}

void
testApWorks()
{
  // Arrange
  auto f = [](char c) {
    return toupper(c);
  };
  auto parser = pure(f) * charP('a');

  // Act
  auto result = parser.run("aoc");

  // Assert
  assert(result.isSuccess());
  assert(result.value().first == 'A');
  assert(result.value().second == "oc");
}

void
testSequenceTransformsAListOfCharPIntoAListOfCharacters()
{
  auto parsers = std::vector{ charP('a'), charP('o'), charP('c') };
  auto parser = sequence(parsers);

  auto result = parser.run("aoc");

  assert(result.isSuccess());
  assert(result.value().first == std::vector({ 'a', 'o', 'c' }));
  assert(result.value().second == "");
}

auto
main() -> int
{
  testCharPCanParseASingleCharacter();
  testCharPFailsToParseANonMatchingString();
  testSequenceCanParsePartOfItsInput();
  testOrCanParseWhenGivenValidAlternatives();
  testChoiceCanParseWhenGivenValidAlternatives();
  testChoiceFailsWhenNoneOfItsParsersCanParseTheInput();
  testAnyOfCanParseWhenGivenValidAlternatives();
  testMappingAParserWorks();
  testApWorks();
  testSequenceTransformsAListOfCharPIntoAListOfCharacters();
  return 0;
}
