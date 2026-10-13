#include "Krystal.Booey/CSS/Properties/Consumers/Percentage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::CssText;
  using css::tests::ValueFixture;

  namespace
  {
    struct Tokens
    {
      InputStream Input;
      Tokenizer Tokenizer;

      explicit Tokens(utf8_stringview css) : Input(CSSOMString(css)), Tokenizer(Input)
      {
        REQUIRE(Tokenizer.PumpTokenizer());
      }
    };

    std::string Consume(utf8_stringview css)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      auto value = ConsumePercentageDividedBy100OrNumber(range, state);
      if (!value)
      {
        return "<null>";
      }

      CHECK(range.IsAtEnd());
      return CssText(*value);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ConsumePercentageDividedBy100OrNumber)",
                   "[CSS][Properties][Consumers]")
  {
    CHECK(Consume(u8"0.25") == "0.25");
    CHECK(Consume(u8"25%") == "0.25");
    CHECK(Consume(u8"calc(1 + 2)") == "calc(3)");
    CHECK(Consume(u8"5px") == "<null>");
    CHECK(Consume(u8"auto") == "<null>");
  }

  // FIXME: calc() producing a percentage isn't divided by 100 (also upstream).
  TEST_CASE_METHOD(ValueFixture,
                   "PropertyParserHelpers(ConsumePercentageDividedBy100OrNumber calc percentage)",
                   "[CSS][Properties][Consumers][!shouldfail]")
  {
    CHECK(Consume(u8"calc(25%)") == "0.25");
  }
}
