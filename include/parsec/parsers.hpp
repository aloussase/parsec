#pragma once

#include <cctype>

#include "parsec.hpp"

namespace parsec
{

/**
 * Parse any one character.
 */
static inline Parser<char>
anyChar()
{
  return satisfy([]([[maybe_unused]] char c) { return true; }, "any character");
}

/**
 * Parse any character but the provided one.
 */
static inline Parser<char>
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
static inline Parser<char>
digit()
{
  return satisfy(isdigit, "digit");
}

/**
 * Parse a sequence of digits into a string.
 */
static inline Parser<std::string>
digits()
{
  return many1(digit()) & [](auto digits) {
    return std::string(digits.begin(), digits.end());
  };
}

/**
 * Parse and decode an unsigned decimal number.
 */
static inline auto
decimal()
{
  return digits() & [](const std::string& digits) {
    return std::stoi(digits);
  };
}

/**
 * Parse an ascii letter, as per isalpha.
 */
static inline auto
letter()
{
  return satisfy(isalpha, "letter");
}

/**
 * Parse a space character, as per isspace.
 */
static inline auto
space()
{
  return satisfy(isspace, "space");
}

}
