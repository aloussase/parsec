#pragma once

#include <utility>

namespace parsec
{

// FIXME: There must be a way to create a curry<N> template to
// curry arbitrary functions.

template <typename F>
auto
curry1(F f)
{
  return [f](auto x) {
    return f(std::move(x));
  };
}

template <typename F>
auto
curry2(F f)
{
  return [f](auto x) {
    return [f, x](auto y) {
      return f(std::move(x), std::move(y));
    };
  };
}

template <typename F>
auto
curry3(F f)
{
  return [f](auto x) {
    return [f, x](auto y) {
      return [f, x, y](auto z) {
        return f(std::move(x), std::move(y), std::move(z));
      };
    };
  };
}

}
