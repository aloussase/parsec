#pragma once

#include <utility>

namespace parsec
{

// FIXME: There must be a way to create a curry<N> template to
// curry arbitrary functions.

/**
 * C++ functor that can construct instances of any class
 * given the right arguments.
 */
template <typename Class>
class construct
{
public:
  template <typename... Args>
  constexpr auto
  operator()(Args&&... args) const
  {
    return Class{ std::forward<Args>(args)... };
  }
};

template <typename F>
auto
curry1(F f)
{
  return [f](auto x) {
    return f(x);
  };
}

template <typename F>
auto
curry2(F f)
{
  return [f](auto x) {
    return [f, x](auto y) {
      return f(x, y);
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
        return f(x, y, z);
      };
    };
  };
}

}
