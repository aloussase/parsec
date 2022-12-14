//
// Created by aloussase on 12/12/22.
//

#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <list>
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
    return isSuccess() ? std::nullopt : std::optional{ std::get<ParserError>(value_) };
  }

  [[nodiscard]] std::optional<T>
  asOpt() const noexcept
  {
    return isSuccess() ? std::optional{ std::get<T>(value_) } : std::nullopt;
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

/**
 * Get a parser result as an optional value.
 */
template <typename T>
[[nodiscard]] constexpr std::optional<T>
maybeResult(typename Parser<T>::result_type result)
{
  return result.isSuccess() ? std::optional{ result.value().first } : std::nullopt;
}

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
template <typename F, typename T>
[[nodiscard]] constexpr auto
ap(const Parser<F>& fP, const Parser<T>& p) noexcept
{
  return fP >>= [p](F f) {
    return p >>= [f](T x) {
      return pure(f(x));
    };
  };
}

template <typename F, typename T>
[[nodiscard]] constexpr auto
operator*(const Parser<F>& fP, const Parser<T>& p)
{
  return ap(fP, p);
}

/**
 * Monadic bind for parsers.
 */
template <typename F, typename T>
[[nodiscard]] constexpr auto
bind(const Parser<T>& p, F f) noexcept
{
  using result_parser = typename std::result_of<F(T)>::type;
  return result_parser([f, p](const std::string& input) -> result_parser::result_type {
    auto result = p.run(input);
    if (result.isFailure()) return result.error().value();
    auto [value, remainingInput] = result.value();
    return f(value).run(remainingInput);
  });
}

template <typename F, typename T>
[[nodiscard]] constexpr auto
operator>>=(const Parser<T>& p, F f) noexcept
{
  return bind(p, f);
}

/**
 * Apply a function to the value inside a Parser and return its result
 * in the Parser context.
 */
template <typename F, typename T>
[[nodiscard]] constexpr auto
map(F f, const Parser<T>& p) noexcept
{
  return p >>= [f](T value) {
    return pure(f(value));
  };
}

template <typename F, typename T>
[[nodiscard]] constexpr auto
operator%(F f, const Parser<T>& p) noexcept
{
  return map(f, p);
}

/**
 * Reverse functorial application.
 */
template <typename F, typename T>
[[nodiscard]] constexpr auto
operator&(const Parser<T>& p, F f) noexcept
{
  return map(f, p);
}

template <typename T>
[[nodiscard]] constexpr auto
sequence(const std::vector<Parser<T> >& parsers) noexcept
{
  return Parser<std::vector<T> >(
      [parsers](const std::string& input) -> Parser<std::vector<T> >::result_type {
        std::vector<T> results{};
        std::string remaining = input;

        for (const auto& parser : parsers)
          {
            auto result = parser.run(remaining);
            if (result.isFailure()) return result.error().value();
            results.push_back(result.value().first);
            remaining = result.value().second;
          }

        return make_success(results, remaining);
      });
}

/**
 * Parses any character that satisfies the given predicate.
 */
template <typename P>
[[nodiscard]] Parser<char>
satisfy(P pred, const std::string& errmsg = "satisfy: Failed to satisfy predicate") noexcept
{
  return Parser<char>([pred, errmsg](const std::string& input) -> Parser<char>::result_type {
    return pred(input[0]) ? make_success(input[0], input.substr(1)) : ParserError{ errmsg };
  });
}

/**
 * Parse a single character.
 */
Parser<char>
charP(char charToMatch)
{
  return satisfy([charToMatch](char c) {
    return charToMatch == c;
  });
}

/**
 * Parse a string.
 */
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
 * Parse any single one of the specified characters.
 */
[[nodiscard]] Parser<char>
anyOf(const std::initializer_list<char>& chars)
{
  return Parser<char>([chars](const std::string& input) -> Parser<char>::result_type {
    for (auto c : chars)
      {
        auto result = charP(c).run(input);
        if (result.isSuccess()) return result;
      }
    // TODO: Provide better error message;
    return ParserError{ "Failed to match any characters" };
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
[[nodiscard]] constexpr auto
many(const Parser<T>& parser) noexcept
{
  return Parser<std::list<T> >(
      [parser](const std::string& input) -> Parser<std::list<T> >::result_type {
        std::string remaining = input;
        std::list<T> xs{};

        while (1)
          {
            auto result = parser.run(remaining);
            if (result.isFailure()) return make_success(std::move(xs), remaining);
            xs.push_back(result.value().first);
            remaining = result.value().second;
          }
      });
}

template <typename T>
[[nodiscard]] constexpr Parser<std::list<T> >
many1(const Parser<T>& parser) noexcept
{
  return parser >>= [parser](auto x) {
    return many(parser) & [x](std::list<T> xs) {
      xs.push_front(x);
      return xs;
    };
  };
}

/**
 * Run the provided parser and return the provided default
 * value if it fails.
 */
template <typename T>
[[nodiscard]] constexpr Parser<T>
option(const T& def, Parser<T> parser)
{
  return parser | pure(def);
}

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

/**
 * Applies one or more ocurrences of p, separated by sep.
 * @return A list of the values returned by p.
 */
template <typename T, typename Sep>
[[nodiscard]] constexpr Parser<std::list<T> >
sepBy1(const Parser<T>& p, const Parser<Sep>& sep) noexcept
{
  return p >>= [p, sep](T x) {
    return many(sep >> p) & [x](std::list<T> xs) {
      xs.push_front(x);
      return xs;
    };
  };
}

/**
 * Applies zero or more ocurrences of p, separated by sep.
 * @return A list of the values returned by p.
 */
template <typename T, typename Sep>
[[nodiscard]] constexpr Parser<std::list<T> >
sepBy(const Parser<T>& p, const Parser<Sep>& sep) noexcept
{
  return sepBy1(p, sep) | pure(std::list<T>{});
}

/**
 * Run the second parser, ignoring the result of the first one
 * and returning the result of the second one.
 */
template <typename T, typename R>
[[nodiscard]] constexpr Parser<R>
operator>>(const Parser<T>& p1, const Parser<R>& p2)
{
  return p1 >>= [p2]([[maybe_unused]] T) {
    return p2;
  };
}

/**
 * Run the first parser and then the second, ignoring the
 * result of the second one and returning that of the first
 * one.
 */
template <typename T, typename R>
[[nodiscard]] constexpr Parser<T>
operator<(const Parser<T>& p1, const Parser<R>& p2)
{
  return p1 >>= [p2](T value) -> Parser<T> {
    return p2 & [value]([[maybe_unused]] R) {
      return value;
    };
  };
}

template <typename T, typename R>
[[nodiscard]] constexpr Parser<R>
operator>(const Parser<T>& p1, const Parser<R>& p2)
{
  return p1 >> p2;
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

}
