#include <map>
#include <memory>

#include "parsec/all.hpp"

using namespace parsec;

class JsonValue
{
public:
  ~JsonValue() {}

  virtual std::string toString() const noexcept = 0;
};

template <typename T = JsonValue>
using JsonPtr = std::shared_ptr<T>;

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
  JsonString(const std::string& v)
      : value{ v }
  {
  }

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
  JsonNumber(int v)
      : value{ v }
  {
  }

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
  JsonBool(bool v)
      : value{ v }
  {
  }

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
  JsonObject(std::map<JsonPtr<JsonString>, JsonPtr<> > v)
      : value{ v }
  {
  }

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

  std::map<JsonPtr<JsonString>, JsonPtr<> > value;
};

auto
jsonNullP()
{
  return stringP("null") >> pure(JsonPtr<>{ new JsonNull });
}

auto
jsonBoolP()
{
  return stringP("true") >> pure(JsonPtr<>{ new JsonBool(true) })
       | stringP("false") >> pure(JsonPtr<>{ new JsonBool(false) });
}

auto
jsonStringP()
{
  auto parser = charP('"') > many1(notChar('"')) < charP('"');
  return parser & convert::tostring() & [](std::string s) {
    return JsonPtr<>(new JsonString(s));
  };
}

auto
jsonNumberP()
{
  return decimal() & [](int i) {
    return JsonPtr<>(new JsonNumber(i));
  };
}

Parser<JsonPtr<> > jsonValueP() noexcept;

Parser<JsonPtr<> >
jsonObjectP()
{
  auto ws = many(space());

  std::function mkKeyValue = [](JsonPtr<> key, JsonPtr<> value
                             ) -> std::pair<JsonPtr<JsonString>, JsonPtr<> > {
    return std::pair{ std::static_pointer_cast<JsonString>(key), value };
  };

  auto keyValue = curry(mkKeyValue) % (jsonStringP() < (ws > charP(':') > ws))
                * jsonValueP();

  auto keyValues = many1(
      (ws > keyValue < ws) | (ws > charP(',') > keyValue < ws)
  );

  return (charP('{') > keyValues < charP('}')) & [](auto values) {
    std::map<JsonPtr<JsonString>, JsonPtr<> > m{};
    m.insert(values.begin(), values.end());
    return JsonPtr<>(new JsonObject(m));
  };
}

Parser<JsonPtr<> >
jsonValueP() noexcept
{
  return Parser<JsonPtr<> >([](std::string_view input) {
    auto parser = jsonNullP() | jsonNumberP() | jsonStringP() | jsonBoolP()
                | jsonObjectP();
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
      std::cerr << err.show() << "\n";
    }

  return 0;
}
