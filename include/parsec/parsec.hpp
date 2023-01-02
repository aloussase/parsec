//
// Created by aloussase on 12/12/22.
//

#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace parsec
{

class ParserError
{
public:
  [[nodiscard]] static ParserError
  create(const std::string& parser_label, const std::string& errmsg) noexcept
  {
    return ParserError{ parser_label, errmsg };
  }

  [[nodiscard]] std::string
  show() const noexcept
  {
    return parser_label_ + ": " + errmsg_;
  }

private:
  ParserError(std::string parser_label, std::string errmsg)
      : parser_label_{ std::move(parser_label) }, errmsg_{ std::move(errmsg) }
  {
  }

  std::string parser_label_;
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

  // Implicit conversion from ParserError
  ParseResult(ParserError err) : ParseResult{ Failure, std::move(err) } {}

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

  [[nodiscard]] ParserError
  asError() const
  {
    return std::get<ParserError>(value_);
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
  constexpr explicit ParseResult(result_type type, T value)
      : type_{ type }, value_{ value }
  {
  }
  explicit ParseResult(result_type type, ParserError value)
      : type_{ type }, value_{ value }
  {
  }

  result_type type_;
  std::variant<T, ParserError> value_;
};

template <typename T>
class Parser
{
public:
  using result_type = ParseResult<std::pair<T, std::string_view> >;
  using function_type = std::function<result_type(std::string_view)>;

  constexpr
  Parser(const function_type& f)
      : m_label{ "unknown" }, m_parselet{ f }
  {
  }

  constexpr
  Parser(const std::string& label, const function_type& f)
      : m_label{ label }, m_parselet(f)
  {
  }

  constexpr ~Parser() = default;

  [[nodiscard]] constexpr const std::string&
  getLabel() const noexcept
  {
    return m_label;
  }

  constexpr Parser&
  withLabel(const std::string& label) noexcept
  {
    m_label = label;
    return *this;
  }

  [[nodiscard]] constexpr result_type
  run(std::string_view input) const noexcept
  {
    return m_parselet(input);
  }

  [[nodiscard]] constexpr std::optional<T>
  runOptional(std::string_view input) const noexcept
  {
    if (auto result = run(input).asOpt(); result) return result->first;
    return std::nullopt;
  }

  [[nodiscard]] constexpr T
  runThrowing(std::string_view input) const
  {
    return run(input).value().first;
  }

private:
  std::string m_label;
  function_type m_parselet;
};

template <typename T>
constexpr auto
make_success(const T& value, std::string_view input) noexcept
{
  return Parser<T>::result_type::success(std::pair{ value, input });
}

/**
 * Put a value in a Parser context.
 */
template <typename T>
[[nodiscard]] constexpr auto
pure(const T& value)
{
  return Parser<T>([value](std::string_view input) {
    return make_success(value, input);
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
  return result_parser([f, p](std::string_view input) ->
                       typename result_parser::result_type {
                         auto result = p.run(input);
                         if (result.isFailure()) return result.asError();
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
  auto parselet = [parsers](std::string_view input) ->
      typename Parser<std::vector<T> >::result_type {
        std::vector<T> results{};
        std::string_view remaining = input;

        for (const auto& parser : parsers)
          {
            auto result = parser.run(remaining);
            if (result.isFailure()) return result.asError();
            results.push_back(result.value().first);
            remaining = result.value().second;
          }

        return make_success(results, remaining);
      };
  return Parser<std::vector<T> >(parselet);
}

/**
 * Parses any character that satisfies the given predicate.
 */
template <typename P>
[[nodiscard]] auto
satisfy(P predicate, const std::string& label) noexcept
{
  auto parselet = [predicate,
                   label](std::string_view input) -> Parser<char>::result_type {
    if (input.empty()) return ParserError::create(label, "Empty input!");
    if (predicate(input[0])) return make_success(input[0], input.substr(1));
    return ParserError::create(label,
                               std::string("Unexpected '") + input[0] + "'");
  };
  return Parser<char>(parselet);
}

/**
 * Parse a single character.
 */
[[nodiscard]] static inline Parser<char>
charP(char charToMatch) noexcept
{
  return satisfy(
      [charToMatch](char c) {
        return charToMatch == c;
      },
      std::string("character '") + charToMatch + "'");
}

/**
 * Parse a string.
 */
[[nodiscard]] static inline Parser<std::string>
stringP(const std::string& s)
{
  auto label = "string \"" + s + "\"";
  return Parser<std::string>(
      label,
      [s, label](std::string_view input) -> Parser<std::string>::result_type {
        if (auto pos = input.find(s); pos == 0)
          return make_success(s, input.substr(pos + s.length()));
        return ParserError::create(label, "Failed to parse string");
      });
}

/**
 * Return the result of the first parser that succeeds.
 */
template <typename... Parsers>
[[nodiscard]] constexpr auto
choice(Parsers&&... parsers)
{
  return (... | std::forward<Parsers>(parsers));
}

/**
 * Parse any single one of the specified characters.
 */
template <typename... Chars>
[[nodiscard]] constexpr auto
anyOf(Chars&&... chars)
{
  return choice(charP(std::forward<Chars>(chars))...)
      .withLabel((std::string("any of: ") + ... + chars));
}

template <typename T>
[[nodiscard]] constexpr auto
many(const Parser<T>& parser) noexcept
{
  return Parser<std::list<T> >(
      std::string("many of ") + parser.getLabel(),
      [parser](std::string_view input) {
        std::string_view remaining = input;
        std::list<T> xs{};
        while (1)
          {
            auto result = parser.run(remaining);
            if (result.isFailure())
              return make_success(std::move(xs), std::move(remaining));
            xs.push_back(std::move(result.value().first));
            remaining = std::move(result.value().second);
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
  return (parser | pure(def))
      .withLabel(std::string("Optional ") + parser.getLabel());
}

/**
 * Applies one or more ocurrences of p, separated by sep.
 * @return A list of the values returned by p.
 */
template <typename T, typename Sep>
[[nodiscard]] constexpr Parser<std::list<T> >
sepBy1(const Parser<T>& p, const Parser<Sep>& sep) noexcept
{
  return (p >>=
          [p, sep](T x) {
            return many(sep >> p) & [x](std::list<T> xs) {
              xs.push_front(x);
              return xs;
            };
          })
      .withLabel(p.getLabel() + " separated by " + sep.getLabel());
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
 * Return a parser that parses characters a long as the provided predicate holds
 * true.
 */
template <typename Pred>
[[nodiscard]] constexpr auto
takeWhile(Pred predicate) noexcept
{
  return Parser<std::vector<char> >(
      "takeWhile",
      [predicate](
          std::string_view input) -> Parser<std::vector<char> >::result_type {
        std::string::size_type i = 0;
        std::vector<char> result{};

        for (; predicate(input[i]) && i < input.length(); i++)
          result.push_back(input[i]);

        return make_success(result, input.substr(i + 1));
      });
}

/**
 * Returns a parser that skips characters for as long as the provided predicate
 * holds true.
 *
 * NOTE: The value produced by the returned parser should not be used and is
 * subject to change.
 *
 */
template <typename Pred>
[[nodiscard]] constexpr auto
skipWhile(Pred predicate) noexcept
{
  return takeWhile(predicate) >> pure(std::nullopt);
}

/**
 * Run the second parser, ignoring the result of the first one
 * and returning the result of the second one.
 */
template <typename T, typename R>
[[nodiscard]] constexpr Parser<R>
operator>>(const Parser<T>& p1, const Parser<R>& p2)
{
  return (p1 >>=
          [p2]([[maybe_unused]] T) {
            return p2;
          })
      .withLabel(p1.getLabel() + " and then " + p2.getLabel());
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
  auto label = p1.getLabel() + " or " + p2.getLabel();
  return Parser<T>(label, [p1, p2](std::string_view input) {
    auto result = p1.run(input);
    return result.isSuccess() ? result : p2.run(input);
  });
}

}
