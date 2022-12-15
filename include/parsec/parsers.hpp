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
  return satisfy([]([[maybe_unused]] char c) { return true; }, "any character");
}

/**
 * Parse any character but the provided one.
 */
Parser<char>
notChar(char charToNotMatch)
{
  // clang-format off
  return satisfy([charToNotMatch](char c) { return c != charToNotMatch; },
                 "not char '"s + charToNotMatch + "'");
  // clang-format on
}

/**
 * Parse a single digit.
 */
Parser<char>
digit()
{
  return satisfy(isdigit, "digit");
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
  return satisfy(isalpha, "letter");
}

/**
 * Parse a space character, as per isspace.
 */
auto
space()
{
  return satisfy(isspace, "space");
}

}
