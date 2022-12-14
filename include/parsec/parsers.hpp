#pragma once

#include <cctype>

#include "parsec.hpp"

namespace parsec
{

/**
 * Parse a single digit.
 */
Parser<char>
digitP()
{
  return satisfy(isdigit, "Failed to parse digit");
}

/**
 * Parse a sequence of digits into a string.
 */
Parser<std::string>
digitsP()
{
  return many1(digitP()) & [](auto digits) {
    return std::string(digits.begin(), digits.end());
  };
}

/**
 * Parse and decode an unsigned decimal number.
 */
auto
decimalP()
{
  return digitsP() & [](const std::string& digits) {
    return std::stoi(digits);
  };
}

/**
 * Parse an ascii letter, as per isalpha.
 */
auto
letterP()
{
  return satisfy(isalpha, "Failed to match letter");
}

/**
 * Parse a space character, as per isspace.
 */
auto
spaceP()
{
  return satisfy(isspace, "Failed to match space");
}

}
