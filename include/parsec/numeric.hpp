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
  return anyOf({ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' });
}

Parser<std::list<char> >
digitsP()
{
  return many1(digitP());
}

}
