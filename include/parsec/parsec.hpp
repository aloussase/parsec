//
// Created by aloussase on 12/12/22.
//

#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace parsec
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

  ParseResult(const ParserError& err) : ParseResult{ Failure, err } {}

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

private:
  function_type parse_fn;
};

template <typename T>
typename Parser<T>::result_type
make_success(const T& value, const std::string& input) noexcept
{
  return Parser<T>::result_type::success({ value, input });
}

template <typename T>
typename Parser<T>::result_type
make_failure(const std::string& msg) noexcept
{
  return Parser<T>::result_type::failure(ParserError{ msg });
}

/**
 * Put a value in a Parser context.
 */
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

/**
 * Function application in the Parser context.
 */
template <typename T, typename R>
[[nodiscard]] constexpr auto
ap(const Parser<T>& p1, const Parser<R>& p2) noexcept
{
  using return_type = typename std::result_of<T(R)>::type;

  return Parser<return_type>([p1, p2](const std::string& input) {
    auto result1 = p1.run(input);
    if (result1.isFailure())
      return Parser<return_type>::result_type::failure(result1.error().value());

    auto [fn, remaining] = result1.value();

    auto result2 = p2.run(remaining);
    if (result2.isFailure())
      return Parser<return_type>::result_type::failure(result2.error().value());

    auto [value, remaining2] = result2.value();

    return Parser<return_type>::result_type::success({
        fn(value),
        remaining2,
    });
  });
}

/**
 * Apply a function to the value inside a Parser and return its result
 * in the Parser context.
 */
template <typename F, typename T>
[[nodiscard]] constexpr auto
map(F f, const Parser<T>& p) noexcept
{
  using return_type = typename std::result_of<F(T)>::type;

  return Parser<return_type>([f, p](const std::string& input) {
    auto result = p.run(input);
    if (result.isFailure())
      return Parser<return_type>::result_type::failure(result.error().value());
    auto [value, remaining] = result.value();
    return Parser<return_type>::result_type::success({ f(value), remaining });
  });
}

template <typename T>
[[nodiscard]] constexpr auto
sequence(const std::vector<Parser<T> >& parsers) noexcept
{
  return Parser<std::vector<T> >([parsers](const std::string& input) {
    std::vector<T> results{};
    std::string remaining = input;

    for (const auto& parser : parsers)
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

/**
 * Parse a single character.
 */
Parser<char>
charP(char c)
{
  return Parser<char>([c](const std::string& input) {
    if (input[0] == c) return Parser<char>::result_type::success({ input[0], input.substr(1) });
    auto errmsg = std::string{ "Failed to match character: " } + c;
    return Parser<char>::result_type::failure(ParserError{ errmsg });
  });
}

Parser<std::string>
stringP(const std::string& s)
{
  return Parser<std::string>([s](const std::string& input) -> Parser<std::string>::result_type {
    if (auto pos = input.find(s); pos != std::string::npos)
      return make_success(s, input.substr(pos + s.length()));
    return ParserError{ "Failed to parse string" };
  });
}

/**
 * Parses any character that satisfies the given predicate.
 */
template <typename P>
[[nodiscard]] Parser<char>
satisfy(P pred) noexcept
{
  return Parser<char>([pred](const std::string& input) -> Parser<char>::result_type {
    if (pred(input[0])) return make_success(input[0], input.substr(1));
    return ParserError{ "Failed to satisfy predicate" };
  });
}

/**
 * Parse any single one of the specified characters.
 */
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

/**
 * Return the result of the first parser that succeeds.
 */
template <typename T>
[[nodiscard]] constexpr Parser<T>
choice(const std::initializer_list<Parser<T> >& parsers)
{
  return Parser<T>([parsers](const std::string& input) -> Parser<T>::result_type {
    for (auto parser : parsers)
      {
        auto result = parser.run(input);
        if (result.isSuccess()) return result;
      }

    return ParserError{
      "Failed to match any parsers in choice",
    };
  });
}

template <typename T>
[[nodiscard]] constexpr Parser<std::vector<T> >
many(const Parser<T>& parser) noexcept
{
  return Parser<std::vector<T> >(
      [parser](const std::string& input) -> Parser<std::vector<T> >::result_type {
        std::string remaining = input;
        std::vector<T> vs{};

        while (1)
          {
            auto result = parser.run(remaining);
            if (result.isFailure()) return make_success(std::move(vs), remaining);
            vs.push_back(result.value().first);
            remaining = result.value().second;
          }
      });
}

template <typename T>
[[nodiscard]] constexpr Parser<std::vector<T> >
many1(const Parser<T>& parser) noexcept
{
  return Parser<std::vector<T> >(
      [parser](const std::string& input) -> Parser<std::vector<T> >::result_type {
        // TODO: We could use bind here.
        auto result = parser.run(input);

        if (result.isFailure()) return ParserError{ "many1: Failed to match" };

        std::vector<T> vs{ result.value().first };
        auto [results, remaining] = many(parser).run(input).value();

        std::move(results.begin(), results.end(), vs.begin() + 1);

        return make_success(std::move(vs), remaining);
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
  return ap(p1, p2);
}

template <typename F, typename T>
[[nodiscard]] constexpr auto
operator%(F f, const Parser<T>& p) noexcept
{
  return map(f, p);
}

}
