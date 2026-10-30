#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::Tokens;

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyle)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"decimal", ConsumeCounterStyle) == "decimal");
    CHECK(ConsumeToText(u8"my-style", ConsumeCounterStyle) == "my-style");

    // Predefined counter style names are case-insensitive.
    CHECK(ConsumeToText(u8"DECIMAL", ConsumeCounterStyle) == "decimal");

    CHECK(ConsumeToText(u8"none", ConsumeCounterStyle) == "<null>");
    CHECK(ConsumeToText(u8"inherit", ConsumeCounterStyle) == "<null>");
    CHECK(ConsumeToText(u8"\"x\"", ConsumeCounterStyle) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyleName)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"decimal", ConsumeCounterStyleName) == "decimal");
    CHECK(ConsumeToText(u8"Decimal", ConsumeCounterStyleName) == "decimal");
    CHECK(ConsumeToText(u8"my-style", ConsumeCounterStyleName) == "my-style");
    CHECK(ConsumeToText(u8"none", ConsumeCounterStyleName) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyleNameInPrelude)", "[CSS][Properties][Consumers]")
  {
    auto prelude = [](utf8_stringview css)
    {
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return ConsumeCounterStyleNameInPrelude(range);
    };

    CHECK(prelude(u8"my-style") == CSSOMStringAtom(u8"my-style"));
    CHECK(prelude(u8"none") == CSSOMStringAtom::Null());
    CHECK(prelude(u8"my-style extra") == CSSOMStringAtom::Null());

    // The predefined styles that can't be redefined.
    CHECK(prelude(u8"decimal") == CSSOMStringAtom::Null());
    CHECK(prelude(u8"disc") == CSSOMStringAtom::Null());

    // Other predefined styles can be, and their names are lowercased.
    CHECK(prelude(u8"Upper-Roman") == CSSOMStringAtom(u8"upper-roman"));
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyleSystem)", "[CSS][Properties][Consumers]")
  {
    for (auto system : {u8"cyclic", u8"fixed", u8"symbolic", u8"additive", u8"alphabetic", u8"numeric"})
    {
      CHECK(ConsumeToText(system, ConsumeCounterStyleSystem) == css::tests::ToNarrow(system));
    }

    CHECK(ConsumeToText(u8"fixed 3", ConsumeCounterStyleSystem) == "fixed 3");
    CHECK(ConsumeToText(u8"extends decimal", ConsumeCounterStyleSystem) == "extends decimal");

    CHECK(ConsumeToText(u8"extends none", ConsumeCounterStyleSystem) == "<null>");
    CHECK(ConsumeToText(u8"cyclic 3", ConsumeCounterStyleSystem) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyleRange)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeCounterStyleRange) == "auto");
    CHECK(ConsumeToText(u8"1 5", ConsumeCounterStyleRange) == "1 5");
    CHECK(ConsumeToText(u8"infinite 10", ConsumeCounterStyleRange) == "infinite 10");
    CHECK(ConsumeToText(u8"1 infinite, 20 30", ConsumeCounterStyleRange) == "1 infinite, 20 30");

    // The lower bound can't exceed the upper bound.
    CHECK(ConsumeToText(u8"5 1", ConsumeCounterStyleRange) == "<null>");
    CHECK(ConsumeToText(u8"1", ConsumeCounterStyleRange) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterStyleAdditiveSymbols)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"1 a", ConsumeCounterStyleAdditiveSymbols) == "1 a");
    CHECK(ConsumeToText(u8"\"x\" 5", ConsumeCounterStyleAdditiveSymbols) == "5 \"x\"");
    CHECK(ConsumeToText(u8"10 x, 5 y, 1 z", ConsumeCounterStyleAdditiveSymbols) == "10 x, 5 y, 1 z");

    // Weights must be non-negative and strictly descending.
    CHECK(ConsumeToText(u8"1 a, 5 b", ConsumeCounterStyleAdditiveSymbols) == "<null>");
    CHECK(ConsumeToText(u8"-1 a", ConsumeCounterStyleAdditiveSymbols) == "<null>");
    CHECK(ConsumeToText(u8"1 a b", ConsumeCounterStyleAdditiveSymbols) == "<null>");
  }
}
