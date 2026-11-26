#include "Krystal.Booey/Css/Properties/Consumers/ColorAdjust.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"
#include "Krystal.Booey/Css/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/Css/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/Css/Values/ColorSchemeValue.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include <string>

namespace krys::boo::css::tests
{
  using parse_property::ConsumeColorScheme;
  using parse_property::ParseUnresolvedColorScheme;

  namespace
  {
    std::string SerialisedColorScheme(utf8_stringview css)
    {
      auto scheme = ParseUnresolvedColorScheme(css, ParserContext {ParserMode::HTMLStandard});
      if (!scheme)
      {
        return "<invalid>";
      }

      auto serialised = Serialise(DefaultSerialisationContext(), *scheme);
      return std::string(serialised.begin(), serialised.end());
    }
  }

  TEST_CASE("Colour scheme parsing", "[CSS][Properties][Consumers][ColorScheme]")
  {
    auto normal = ParseUnresolvedColorScheme(u8"normal", ParserContext {ParserMode::HTMLStandard});
    REQUIRE(normal);
    CHECK(normal->IsNormal());
    CHECK_FALSE(normal->Only);

    auto schemes = ParseUnresolvedColorScheme(u8"  light dark  ", ParserContext {ParserMode::HTMLStandard});
    REQUIRE(schemes);
    REQUIRE(schemes->Schemes.size() == 2);
    CHECK(schemes->Schemes.Value[0] == u8"light");
    CHECK(schemes->Schemes.Value[1] == u8"dark");
    CHECK_FALSE(schemes->Only);

    CHECK(SerialisedColorScheme(u8"normal") == "normal");
    CHECK(SerialisedColorScheme(u8"light dark") == "light dark");
    CHECK(SerialisedColorScheme(u8"only light") == "light only");
    CHECK(SerialisedColorScheme(u8"dark only") == "dark only");
    CHECK(SerialisedColorScheme(u8"my-scheme") == "my-scheme");

    // `normal` must be alone, `only` must be first or last and needs at least one scheme.
    CHECK(SerialisedColorScheme(u8"normal light") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"light normal") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"only") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"only light only") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"light only dark") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"light, dark") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"light inherit") == "<invalid>");
    CHECK(SerialisedColorScheme(u8"") == "<invalid>");
  }

  TEST_CASE("Colour scheme parsing(Value entry point)", "[CSS][Properties][Consumers][ColorScheme]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    auto consume = [&](utf8_stringview css) -> RefPtr<Value>
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      PropertyParserState state {.Context = context};
      auto result = ConsumeColorScheme(tokens, state);
      if (!tokens.IsAtEnd())
      {
        return nullptr;
      }
      return result;
    };

    auto value = consume(u8"only light dark");
    REQUIRE(value);
    CHECK(Is<ColorSchemeValue>(*value));
    CHECK(CssText(*value) == "light dark only");

    CHECK_FALSE(consume(u8"normal light"));
  }
}
