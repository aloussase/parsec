//
// Created by aloussase on 12/12/22.
//

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace aoc::parser
{

class ParserError
{
public:
  ParserError(const std::string& errmsg) : errmsg_{ errmsg } {}

  [[nodiscard]] std::string
  msg() const noexcept
  {
    return errmsg_;
  }

private:
  std::string errmsg_;
};

template <typename T>
class ParseResult
{
public:
  enum result_type
  {
    Success,
    Failure,
  };

  [[nodiscard]] constexpr bool
  isSuccess() const noexcept
  {
    return type_ == Success;
  }

  [[nodiscard]] constexpr bool
  isFailure() const noexcept
  {
    return type_ == Failure;
  }

  [[nodiscard]] constexpr T
  value() const
  {
    if (isFailure()) throw std::get<ParserError>(value_);
    return std::get<T>(value_);
  }

  [[nodiscard]] std::optional<ParserError>
  error() const
  {
    if (isSuccess()) return std::nullopt;
    return std::get<ParserError>(value_);
  }

  static ParseResult
  failure(const ParserError& errmsg)
  {
    return ParseResult{ Failure, errmsg };
  }

  constexpr static ParseResult
  success(T&& value)
  {
    return ParseResult{ Success, std::forward<T>(value) };
  }

private:
  constexpr explicit ParseResult(result_type type, T&& value)
      : type_{ type }, value_{ std::forward<T>(value) }
  {
  }
  explicit ParseResult(result_type type, ParserError value) : type_{ type }, value_{ value } {}

  result_type type_;
  std::variant<T, ParserError> value_;
};

template <typename T>
class Parser
{
public:
  using result_type = ParseResult<std::pair<T, std::string> >;
  using function_type = std::function<result_type(const std::string&)>;

  constexpr explicit Parser(function_type f) : parse_fn(std::move(f)) {}

  constexpr ~Parser() {}

  [[nodiscard]] constexpr result_type
  run(const std::string& input) const noexcept
  {
    return parse_fn(input);
  }

  /**
   * For ap, the type we produce must be a function type.
   */
  template <typename R>
  [[nodiscard]] constexpr auto
  ap(Parser<R> p) const noexcept
  {
    using return_type = typename std::result_of<T(R)>::type;

    return Parser<return_type>([this, p](const std::string& input) {
      auto result1 = this->run(input);
      if (result1.isFailure())
        return Parser<return_type>::result_type::failure(result1.error().value());

      auto [fn, remaining] = result1.value();

      auto result2 = p.run(remaining);
      if (result2.isFailure())
        return Parser<return_type>::result_type::failure(result2.error().value());

      auto [value, remaining2] = result2.value();

      return Parser<return_type>::result_type::success({
          fn(value),
          remaining2,
      });
    });
  }

  // TODO: Maybe use op & or ^ as infix map.
  template <typename F>
  [[nodiscard]] constexpr auto
  map(F f) const noexcept
  {
    using return_type = typename std::result_of<F(T)>::type;

    return Parser<return_type>([this, f](const std::string& input) {
      auto result = this->run(input);
      if (result.isFailure())
        return Parser<return_type>::result_type::failure(result.error().value());
      auto [value, remaining] = result.value();
      return Parser<return_type>::result_type::success({ f(value), remaining });
    });
  }

private:
  function_type parse_fn;
};

template <typename T>
[[nodiscard]] constexpr auto
sequence(const std::vector<Parser<T> >& parsers) noexcept
{
  return Parser<std::vector<T> >([parsers](const std::string& input) {
    std::vector<T> results{};
    std::string remaining = input;

    for (auto parser : parsers)
      {
        auto result = parser.run(remaining);

        if (result.isFailure())
          return Parser<std::vector<T> >::result_type::failure(result.error().value());

        results.push_back(result.value().first);
        remaining = result.value().second;
      }

    return Parser<std::vector<T> >::result_type::success({ results, remaining });
  });
}

template <typename T>
[[nodiscard]] constexpr Parser<T>
pure(T value)
{
  return Parser<T>([value](const std::string& input) {
    return Parser<T>::result_type::success({
        value,
        input,
    });
  });
}

Parser<char>
charP(char c)
{
  return Parser<char>([c](const std::string& input) {
    if (input[0] == c) return Parser<char>::result_type::success({ input[0], input.substr(1) });
    auto errmsg = std::string{ "Failed to match character: " } + c;
    return Parser<char>::result_type::failure(ParserError{ errmsg });
  });
}

[[nodiscard]] Parser<char>
anyOf(const std::initializer_list<char>& chars)
{
  return Parser<char>([chars](const std::string& input) {
    for (auto c : chars)
      {
        auto parser = charP(c);
        auto result = parser.run(input);
        if (result.isSuccess()) return result;
      }
    // TODO: Provide better error message;
    return Parser<char>::result_type::failure(ParserError{
        "Failed to match any characters",
    });
  });
}

template <typename T>
[[nodiscard]] constexpr Parser<T>
choice(const std::initializer_list<Parser<T> >& parsers)
{
  return Parser<T>([parsers](const std::string& input) {
    for (auto parser : parsers)
      {
        auto result = parser.run(input);
        if (result.isSuccess()) return result;
      }
    return Parser<T>::result_type::failure(ParserError{
        "Failed to match any parsers in choice",
    });
  });
}

/**
 * Run the second parser, ignoring the result of the first one.
 */
template <typename T, typename R>
[[nodiscard]] constexpr Parser<R>
operator>>(const Parser<T>& p1, const Parser<R>& p2)
{
  return Parser<R>([p1, p2](const std::string& input) {
    auto result = p1.run(input);
    if (result.isFailure()) return Parser<R>::result_type::failure(result.error().value());
    auto [_, remaining_input] = result.value();
    return p2.run(remaining_input);
  });
}

/**
 * Run the second parser if the first one fails.
 */
template <typename T>
[[nodiscard]] constexpr Parser<T>
operator|(const Parser<T>& p1, const Parser<T>& p2)
{
  return Parser<T>([p1, p2](const std::string& input) {
    auto result = p1.run(input);
    if (result.isSuccess()) return result;
    return p2.run(input);
  });
}

template <typename F, typename T>
[[nodiscard]] constexpr auto
operator*(const Parser<F>& p1, const Parser<T>& p2)
{
  return p1.ap(p2);
}

}
