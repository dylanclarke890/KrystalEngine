#include "Krystal.Booey/CSS/Parser/SupportsParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD SupportsParser::SupportsResult
      Supports(utf8_stringview condition,
               SupportsParser::ParsingMode mode = SupportsParser::ParsingMode::ForAtRuleSupports)
    {
      auto context = DeclarationTestContext();
      return SupportsParser::SupportsCondition(condition, context, mode);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "SupportsParser: declarations", "[CSS][SupportsParser]")
  {
    CHECK(Supports(u8"(display: grid)") == SupportsParser::Supported);
    CHECK(Supports(u8"  ( display : grid ) ") == SupportsParser::Supported);
    CHECK(Supports(u8"(display: grid !important)") == SupportsParser::Supported);
    CHECK(Supports(u8"(--custom: anything)") == SupportsParser::Supported);
    CHECK(Supports(u8"(display: foo)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(foo: bar)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(display)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"display: grid") == SupportsParser::Invalid);
  }

  TEST_CASE_METHOD(ValueFixture, "SupportsParser: operators", "[CSS][SupportsParser]")
  {
    CHECK(Supports(u8"not (display: foo)") == SupportsParser::Supported);
    CHECK(Supports(u8"NOT (display: grid)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(display: grid) and (color: red)") == SupportsParser::Supported);
    CHECK(Supports(u8"(display: grid) and (color: foo)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(display: foo) or (color: red)") == SupportsParser::Supported);
    CHECK(Supports(u8"(display: foo) OR (color: foo)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"((display: grid) and (color: red)) or (foo: bar)") == SupportsParser::Supported);

    // Mixing operators, a missing operand or missing whitespace is invalid.
    CHECK(Supports(u8"(display: grid) and (color: red) or (foo: bar)") == SupportsParser::Invalid);
    CHECK(Supports(u8"(display: grid) and") == SupportsParser::Invalid);
    CHECK(Supports(u8"(display: grid) and(color: red)") == SupportsParser::Invalid);
    CHECK(Supports(u8"not (display: grid) and (color: red)") == SupportsParser::Invalid);
    CHECK(Supports(u8"(display: grid) xor (color: red)") == SupportsParser::Invalid);
  }

  TEST_CASE_METHOD(ValueFixture, "SupportsParser: general enclosed", "[CSS][SupportsParser]")
  {
    CHECK(Supports(u8"(foo)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(foo bar baz)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"foo(bar)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"(display: grid) or foo(bar)") == SupportsParser::Supported);
    // `not(` is a function token, so this is general enclosed rather than a negation.
    CHECK(Supports(u8"not(display: grid)") == SupportsParser::Unsupported);
  }

  TEST_CASE_METHOD(ValueFixture, "SupportsParser: functions", "[CSS][SupportsParser]")
  {
    CHECK(Supports(u8"selector(a > b)") == SupportsParser::Supported);
    CHECK(Supports(u8"selector(:hover)") == SupportsParser::Supported);
    CHECK(Supports(u8"selector(:unknown)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"selector(a, b)") == SupportsParser::Unsupported);

    CHECK(Supports(u8"font-format(woff2)") == SupportsParser::Supported);
    CHECK(Supports(u8"font-format(collection)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"font-format(\"woff2\")") == SupportsParser::Unsupported);

    CHECK(Supports(u8"font-tech(color-COLRv1)") == SupportsParser::Supported);
    CHECK(Supports(u8"font-tech(variations)") == SupportsParser::Supported);
    CHECK(Supports(u8"font-tech(features-graphite)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"font-tech(incremental)") == SupportsParser::Unsupported);
    CHECK(Supports(u8"font-tech(palettes, variations)") == SupportsParser::Unsupported);
  }

  TEST_CASE_METHOD(ValueFixture, "SupportsParser: CSS.supports mode", "[CSS][SupportsParser]")
  {
    constexpr auto mode = SupportsParser::ParsingMode::AllowBareDeclarationAndGeneralEnclosed;
    CHECK(Supports(u8"display: grid", mode) == SupportsParser::Supported);
    CHECK(Supports(u8"display: foo", mode) == SupportsParser::Unsupported);
    CHECK(Supports(u8"(display: grid)", mode) == SupportsParser::Supported);
    CHECK(Supports(u8"foo", mode) == SupportsParser::Unsupported);
    CHECK(Supports(u8"selector(a)", mode) == SupportsParser::Supported);
  }
}
