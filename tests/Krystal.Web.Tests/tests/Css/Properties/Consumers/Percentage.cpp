#include "Krystal.Web/Css/Properties/Consumers/Percentage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ConsumerTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"

namespace krys::web::css::parse_property::tests
{
  using css::tests::CssText;
  using css::tests::Tokens;
  using css::tests::ValueFixture;

  namespace
  {
    std::string ConsumePercentageText(utf8_stringview css)
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

  TEST_CASE_METHOD(ValueFixture, "parse_property(ConsumePercentageDividedBy100OrNumber)",
                   "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumePercentageText(u8"0.25") == "0.25");
    CHECK(ConsumePercentageText(u8"25%") == "0.25");
    CHECK(ConsumePercentageText(u8"calc(1 + 2)") == "calc(3)");
    CHECK(ConsumePercentageText(u8"5px") == "<null>");
    CHECK(ConsumePercentageText(u8"auto") == "<null>");
  }

  // A calc() percentage keeps its form (its value isn't known until it's resolved); its users divide it by
  // 100 then.
  TEST_CASE_METHOD(ValueFixture, "parse_property(ConsumePercentageDividedBy100OrNumber calc percentage)",
                   "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumePercentageText(u8"calc(25%)") == "calc(25%)");
  }
}
