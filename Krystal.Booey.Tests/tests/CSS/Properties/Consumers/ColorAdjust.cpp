#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include <string>

namespace krys::boo::css::tests
{
  using namespace PropertyParserHelpers;

  namespace
  {
    std::string Serialised(utf8_stringview css)
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
    CHECK(schemes->Schemes.Value[0] == CSSOMStringAtom(u8"light"));
    CHECK(schemes->Schemes.Value[1] == CSSOMStringAtom(u8"dark"));
    CHECK_FALSE(schemes->Only);

    CHECK(Serialised(u8"normal") == "normal");
    CHECK(Serialised(u8"light dark") == "light dark");
    CHECK(Serialised(u8"only light") == "light only");
    CHECK(Serialised(u8"dark only") == "dark only");
    CHECK(Serialised(u8"my-scheme") == "my-scheme");

    // `normal` must be alone, `only` must be first or last and needs at least one scheme.
    CHECK(Serialised(u8"normal light") == "<invalid>");
    CHECK(Serialised(u8"light normal") == "<invalid>");
    CHECK(Serialised(u8"only") == "<invalid>");
    CHECK(Serialised(u8"only light only") == "<invalid>");
    CHECK(Serialised(u8"light only dark") == "<invalid>");
    CHECK(Serialised(u8"light, dark") == "<invalid>");
    CHECK(Serialised(u8"light inherit") == "<invalid>");
    CHECK(Serialised(u8"") == "<invalid>");
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
    CHECK(value->IsColorSchemeValue());
    CHECK(CssText(*value) == "light dark only");

    CHECK_FALSE(consume(u8"normal light"));
  }
}
