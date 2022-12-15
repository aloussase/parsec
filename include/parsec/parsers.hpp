#pragma once

#include <cctype>

#include "parsec.hpp"

namespace parsec
{

/**
 * Parse any one character.
 */
Parser<char>
anyChar()
{
  return satisfy(
      []([[maybe_unused]] char c) {
        return true;
      },
      "Failed to match any char");
}

/**
 * Parse any character but the provided one.
 */
Parser<char>
notChar(char charToNotMatch)
{
  return satisfy(
      [charToNotMatch](char c) {
        return c != charToNotMatch;
      },
      "notAnyCharP: Failed to parse");
}

/**
 * Parse a single digit.
 */
Parser<char>
digit()
{
  return satisfy(isdigit, "Failed to parse digit");
}

/**
 * Parse a sequence of digits into a string.
 */
Parser<std::string>
digits()
{
  return many1(digit()) & [](auto digits) {
    return std::string(digits.begin(), digits.end());
  };
}

/**
 * Parse and decode an unsigned decimal number.
 */
auto
decimal()
{
  return digits() & [](const std::string& digits) {
    return std::stoi(digits);
  };
}

/**
 * Parse an ascii letter, as per isalpha.
 */
auto
letter()
{
  return satisfy(isalpha, "Failed to match letter");
}

/**
 * Parse a space character, as per isspace.
 */
auto
space()
{
  return satisfy(isspace, "Failed to match space");
}

}
