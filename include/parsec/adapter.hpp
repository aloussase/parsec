#pragma once

#include <cstddef>
#include <string>
#include <utility>

namespace parsec
{

// FIXME: There must be a way to create a curry<N> template to
// curry arbitrary functions.

/**
 * Function arity to be used as argument to the curry function template.
 *
 * Currently functions taking up to 5 arguments can be curried
 *
 */
enum class Arity
{
  Unary,
  Binary,
  Ternary,
  NAry4,
  NAry5
};

template <typename Class, Arity Arity>
[[nodiscard]] constexpr auto
make() noexcept
{
  return curry<Arity>([](auto&&... args) {
    return Class(std::forward<decltype(args)>(args)...);
  });
}

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

template <typename F>
auto
curry4(F f)
{
  return [f](auto x) {
    return [f, x](auto y) {
      return [f, x, y](auto z) {
        return [f, x, y, z](auto w) {
          return f(x, y, z, w);
        };
      };
    };
  };
}

template <typename F>
auto
curry5(F f)
{
  return [f](auto x) {
    return [f, x](auto y) {
      return [f, x, y](auto z) {
        return [f, x, y, z](auto w) {
          return [f, x, y, z, w](auto u) {
            return f(x, y, z, w, u);
          };
        };
      };
    };
  };
}

template <Arity N = Arity::Unary, typename F>
constexpr auto
curry(F f)
{
  if constexpr (N == Arity::Unary) return curry1(f);
  if constexpr (N == Arity::Binary) return curry2(f);
  if constexpr (N == Arity::Ternary) return curry3(f);
  if constexpr (N == Arity::NAry4) return curry4(f);
  if constexpr (N == Arity::NAry5) return curry5(f);
}

namespace convert
{

static inline auto
tostring() noexcept
{
  return [](auto it) {
    return std::string{ it.begin(), it.end() };
  };
}

}

}
