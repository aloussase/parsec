#pragma once

#include <cstddef>
#include <string>
#include <utility>

namespace parsec
{

template <size_t N = 1, typename F>
constexpr auto curry(F f);

// FIXME: There must be a way to create a curry<N> template to
// curry arbitrary functions.

template <typename Class, size_t NArgs>
[[nodiscard]] constexpr auto
make() noexcept
{
  // clang-format off
  return curry<NArgs>([](auto&&... args) {
    return Class{ std::forward<decltype(args)>(args)... };
  });
  // clang-format on
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

template <size_t N = 1, typename F>
constexpr auto
curry(F f)
{
  if constexpr (N == 1) return curry1(f);
  if constexpr (N == 2) return curry2(f);
  if constexpr (N == 3) return curry3(f);
  throw "Can't curry";
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
