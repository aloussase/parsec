#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

namespace parsec
{

template <typename Class, typename... Ts>
struct all_args_factory
{
public:
  [[nodiscard]] static constexpr Class
  init(const Ts&... args) noexcept
  {
    // The first argument is the initializer for this class,
    // which is the superclass of Class.
    return Class{ {}, args... };
  }
};

template <typename F>
[[nodiscard]] constexpr auto
curry1(const F& f) noexcept
{
  return [f](const auto& a) {
    return f(a);
  };
}

template <typename F>
[[nodiscard]] constexpr auto
curry2(const F& f) noexcept
{
  return [f](const auto& a) {
    return [f, a](const auto& b) {
      return f(a, b);
    };
  };
}

template <typename F>
[[nodiscard]] constexpr auto
curry3(const F& f) noexcept
{
  return [f](const auto& a) {
    return [f, a](const auto& b) {
      return [f, a, b](const auto& c) {
        return f(a, b, c);
      };
    };
  };
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

} // namespace convert

} // namespace parsec
