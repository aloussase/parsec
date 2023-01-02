#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

namespace parsec
{

template <typename Class>
struct all_args_factory
{
public:
  template <typename... Ts>
  [[nodiscard]] static constexpr Class
  create(const Ts&... args) noexcept
  {
    // The first argument is the initializer for this class,
    // which is the superclass of Class.
    return Class{ {}, args... };
  }
};

template <typename Class, typename... Ts>
auto
make()
{
  return curry(std::function(&Class::template create<Ts...>));
}

template <typename R, typename A>
auto
curry(std::function<R(A)> f)
{
  return [f](A a) {
    return f(a);
  };
}

template <typename R, typename A, typename B>
auto
curry(const std::function<R(A, B)>& f)
{
  return [f](A a) {
    return [f, a](B b) {
      return f(a, b);
    };
  };
}

template <typename R, typename A, typename B, typename C>
auto
curry(std::function<R(A, B, C)> f)
{
  return [f](A a) {
    return [f, a](B b) {
      return [f, a, b](C c) {
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
