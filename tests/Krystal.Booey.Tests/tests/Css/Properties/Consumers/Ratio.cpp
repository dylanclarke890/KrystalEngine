#include "Krystal.Booey/Css/Properties/Consumers/Ratio.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ConsumerTestUtils.hpp"
#include "Krystal.Booey/Css/Values/Primitives/Ratio.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::ConsumeToText;
  using css::tests::Tokens;

  TEST_CASE("parse_property(ConsumeRatio)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"16 / 9", ConsumeRatio) == "16 / 9");
    CHECK(ConsumeToText(u8"16/9", ConsumeRatio) == "16 / 9");
    CHECK(ConsumeToText(u8"2", ConsumeRatio) == "2 / 1");
    CHECK(ConsumeToText(u8"0 / 0", ConsumeRatio) == "0 / 0");
    CHECK(ConsumeToText(u8"calc(1 + 1) / 3", ConsumeRatio) == "calc(2) / 3");

    CHECK(ConsumeToText(u8"-1 / 2", ConsumeRatio) == "<null>");
    CHECK(ConsumeToText(u8"1 / -2", ConsumeRatio) == "<null>");
    CHECK(ConsumeToText(u8"1 /", ConsumeRatio) == "<null>");
    CHECK(ConsumeToText(u8"1px", ConsumeRatio) == "<null>");
  }

  TEST_CASE("parse_property(ConsumeRatioWithBothNumeratorAndDenominator)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"4 / 3", ConsumeRatioWithBothNumeratorAndDenominator) == "4 / 3");
    CHECK(ConsumeToText(u8"4", ConsumeRatioWithBothNumeratorAndDenominator) == "<null>");
  }

  TEST_CASE("parse_property(ConsumeUnresolvedRatio)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};

    // A failed ratio leaves the range untouched.
    Tokens tokens {u8"3 / foo"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(ConsumeUnresolvedRatio(range, state));
    CHECK(range.Peek().Type() == TokenType::Number);

    Tokens single {u8"3 foo"};
    range = single.Tokenizer.Tokens();
    auto ratio = ConsumeUnresolvedRatio(range, state);
    REQUIRE(ratio);
    CHECK(range.Peek().Type() == TokenType::Ident);
  }
}
