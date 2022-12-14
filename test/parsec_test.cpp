//
// Created by aloussase on 12/12/22.
//

#include "parsec/adapter.hpp"
#include "parsec/parsec.hpp"

#include <cassert>

using namespace parsec;

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
  auto parser = choice(charP('a'), charP('o'), charP('c'));

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
  auto parser = choice(charP('a'), charP('o'), charP('o'));

  // Act
  auto result = parser.run("2022");

  // Assert
  assert(result.isFailure());
}

void
testAnyOfCanParseWhenGivenValidAlternatives()
{
  // Arrange
  auto parser = anyOf('a', 'o', 'o');

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
  auto parser = toupper % charP('a');

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
testBuildingASimpleStructWorks()
{
  struct S
  {
    char a, b, c;
  };

  auto mkS = [](char a, char b, char c) {
    return S{ a, b, c };
  };
  auto parser = curry3(mkS) % charP('a') * (charP(' ') >> charP('b')) * (charP(' ') >> charP('c'));

  auto result = parser.run("a b c");

  assert(result.isSuccess());
  assert(result.value().first.a == 'a');
  assert(result.value().first.b == 'b');
  assert(result.value().first.c == 'c');
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

void
testStringPWorksWhenGivenValidInput()
{
  auto parser = stringP("aoc");

  auto result = parser.run("aoc 2022");

  assert(result.isSuccess());
  assert(result.value().first == "aoc");
  assert(result.value().second == " 2022");
}

void
testManyParsesValidNonEmptyInput()
{
  auto parser = many(charP('A'));

  auto result = parser.run("AAA");

  assert(result.isSuccess());
  assert(result.value().first == std::list({ 'A', 'A', 'A' }));
  assert(result.value().second == "");
}

void
testManySucceedsEvenWhenItCantParseAnything()
{
  auto parser = many(charP('a'));

  auto result = parser.run("Advent of Code");

  assert(result.isSuccess());
  assert(result.value().first == std::list<char>());
  assert(result.value().second == "Advent of Code");
}

void
testParsingWhitespace()
{
  auto parser = many(anyOf(' ', '\n', '\t'));

  auto result1 = parser.run("ABC");
  auto result2 = parser.run(" ABC");
  auto result3 = parser.run("\tABC");

  assert(result1.isSuccess() && result2.isSuccess() && result3.isSuccess());
  assert(result1.value().second == "ABC");
  assert(result2.value().first == std::list({ ' ' }));
  assert(result2.value().second == "ABC");
  assert(result3.value().first == std::list({ '\t' }));
  assert(result3.value().second == "ABC");
}

void
testMany1FailsWhenItCanMatchAtLeastOnce()
{
  auto parser = many1(charP('a'));
  auto result = parser.run("Advent of Code");
  assert(result.isFailure());
}

void
testIgnoringTheRightResultWorks()
{
  auto parser = charP('a') < charP('o');
  auto result = parser.run("aoc");
  assert(result.isSuccess());
  assert(result.value().first == 'a');
  assert(result.value().second == "c");
}

void
test_sepBy1_works_with_valid_input()
{
  auto parser = sepBy1(choice(charP('a'), charP('o'), charP('c')), charP(' '));
  auto result = parser.run("a o c");
  assert(result.isSuccess());
  assert(result.value().first == std::list({ 'a', 'o', 'c' }));
  assert(result.value().second == "");
}

void
test_sepBy1_fails_with_invalid_input()
{
  auto parser = sepBy1(choice(charP('a'), charP('o'), charP('c')), charP(' '));
  auto result = parser.run("AOC");
  assert(result.isFailure());
}

void
test_sepBy_works_with_valid_input()
{
  auto parser = sepBy(choice(charP('a'), charP('o'), charP('c')), charP(' '));
  auto result = parser.run("a o c");
  assert(result.isSuccess());
  assert(result.value().first == std::list({ 'a', 'o', 'c' }));
  assert(result.value().second == "");
}

void
test_sepBy_works_with_invalid_input()
{
  auto parser = sepBy(choice(charP('a'), charP('o'), charP('c')), charP(' '));
  auto result = parser.run("AOC");
  assert(result.isSuccess());
  assert(result.value().first == std::list<char>());
  assert(result.value().second == "AOC");
}

auto
main() -> int
{
  // TODO: Change all these to use snake_case.

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
  testBuildingASimpleStructWorks();
  testStringPWorksWhenGivenValidInput();
  testManyParsesValidNonEmptyInput();
  testManySucceedsEvenWhenItCantParseAnything();
  testMany1FailsWhenItCanMatchAtLeastOnce();
  testParsingWhitespace();
  testIgnoringTheRightResultWorks();

  // sepBy1
  test_sepBy1_works_with_valid_input();
  test_sepBy1_fails_with_invalid_input();

  // sepBy
  test_sepBy_works_with_valid_input();
  test_sepBy_works_with_invalid_input();
  return 0;
}
