#pragma once

#include "parsec.hpp"

namespace parsec
{

/**
 * Parse a single digit.
 */
Parser<char>
digitP()
{
  return satisfy(
      [](char c) {
        return c >= '0' && c <= '9';
      },
      "Failed to parse digit");
}

Parser<std::string>
digitsP()
{
  return many1(digitP()) & [](auto digits) {
    return std::string(digits.begin(), digits.end());
  };
}

}
