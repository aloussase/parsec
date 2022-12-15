#include <map>
#include <memory>

#include "parsec/adapter.hpp"
#include "parsec/parsec.hpp"
#include "parsec/parsers.hpp"

using namespace parsec;

class JsonValue
{
public:
  ~JsonValue() {}

  virtual std::string toString() const noexcept = 0;
};

class JsonNull : public JsonValue
{
public:
  std::string
  toString() const noexcept override
  {
    return "null";
  }
};

class JsonString : public JsonValue
{
public:
  JsonString(const std::string& v) : value{ v } {}

  std::string
  toString() const noexcept override
  {
    return '"' + value + '"';
  }

  std::string value;
};

class JsonNumber : public JsonValue
{
public:
  JsonNumber(int v) : value{ v } {}

  std::string
  toString() const noexcept override
  {
    return std::to_string(value);
  }

  int value;
};

class JsonBool : public JsonValue
{
public:
  JsonBool(bool v) : value{ v } {}

  std::string
  toString() const noexcept override
  {
    return value ? "true" : "false";
  }

  bool value;
};

class JsonObject : public JsonValue
{
public:
  JsonObject(std::map<std::shared_ptr<JsonString>, std::shared_ptr<JsonValue> > v) : value{ v } {}

  std::string
  toString() const noexcept override
  {
    std::string result{ "{" };
    for (auto [key, value] : value)
      {
        result += key->toString();
        result += ": ";
        result += value->toString();
        result += ",\n";
      }
    result.pop_back();
    result.pop_back();
    result += "}";
    return result;
  }

  std::map<std::shared_ptr<JsonString>, std::shared_ptr<JsonValue> > value;
};

auto
jsonNullP()
{
  return stringP("null") >> pure(std::shared_ptr<JsonValue>{ new JsonNull });
}

auto
jsonBoolP()
{
  return stringP("true") >> pure(std::shared_ptr<JsonValue>{ new JsonBool(true) })
         | stringP("false") >> pure(std::shared_ptr<JsonValue>{ new JsonBool(false) });
}

auto
jsonStringP()
{
  auto parser = charP('"') > many1(notChar('"')) < charP('"');
  return parser & convert::tostring() & [](std::string s) {
    return std::shared_ptr<JsonValue>(new JsonString(s));
  };
}

auto
jsonNumberP()
{
  return decimal() & [](int i) {
    return std::shared_ptr<JsonValue>(new JsonNumber(i));
  };
}

Parser<std::shared_ptr<JsonValue> > jsonValueP() noexcept;

auto
jsonObjectP()
{
  auto ws = many(space());
  auto mkKeyValue = [](auto key, auto value) {
    return std::pair{ std::static_pointer_cast<JsonString>(key), value };
  };
  auto keyValue = curry<2>(mkKeyValue) % (jsonStringP() < (ws > charP(':') > ws)) * jsonValueP();
  auto keyValues = many1((ws > keyValue < ws) | (ws > charP(',') > keyValue < ws));

  return (charP('{') > keyValues < charP('}')) & [](auto values) {
    std::map<std::shared_ptr<JsonString>, std::shared_ptr<JsonValue> > m{};
    for (auto [key, value] : values)
      m[key] = value;
    return std::shared_ptr<JsonValue>(new JsonObject(m));
  };
}

Parser<std::shared_ptr<JsonValue> >
jsonValueP() noexcept
{
  return Parser<std::shared_ptr<JsonValue> >([](const std::string& input) {
    auto parser = jsonNullP() | jsonNumberP() | jsonStringP() | jsonBoolP() | jsonObjectP();
    return parser.run(input);
  });
}

auto
main() -> int
{
  try
    {
      auto input = "{\"hello\": 12,\"world\": {\"nested\": null}}";
      auto json = jsonValueP().runThrowing(input);
      std::cout << json->toString() << "\n";
    }
  catch (const ParserError& err)
    {
      std::cerr << err.msg() << "\n";
    }

  return 0;
}
