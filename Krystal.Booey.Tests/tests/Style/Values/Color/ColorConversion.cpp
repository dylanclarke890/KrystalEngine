#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/Style/Values/Color/Opacity.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: colours", "[Style][CSSValueConversion][Color]")
  {
    SECTION("Colours")
    {
      CHECK(Convert<Color>(PropertyId::Color, u8"red") == Color {boo::Color::red});
      CHECK(Convert<Color>(PropertyId::Color, u8"#00ff00") == Color {SRGBA<uint8> {0, 255, 0}});
      CHECK(Convert<Color>(PropertyId::Color, u8"currentcolor").IsCurrentColor());
      CHECK_FALSE(IsInvalid());
    }

    SECTION("Keywords")
    {
      CHECK(Convert<Color>(PropertyId::BackgroundColor, u8"transparent")
            == Color {boo::Color::transparentBlack});
    }

    SECTION("A non-colour value is invalid at computed-value time")
    {
      CHECK(ConvertCSSValueToStyle<Color>(*State, *css::PrimitiveValue::Create(3)) == Color {boo::Color {}});
      CHECK(IsInvalid());
    }

    SECTION("Creation resolves currentcolor")
    {
      Style.SetColor(boo::Color::blue);
      CHECK(Created(Color {CurrentColor {}}) == "rgb(0, 0, 255)");
      CHECK(Created(Color {boo::Color::red}) == "rgb(255, 0, 0)");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: dynamic range limits", "[Style][CSSValueConversion]")
  {
    // `dynamic-range-limit` is behind a setting that's off without settings, so the values are built
    // directly.
    CHECK(
      ConvertCSSValueToStyle<DynamicRangeLimit>(*State, *css::PrimitiveValue::Create(css::ValueId::Standard))
      == DynamicRangeLimit {css::keywords::Standard {}});
    CHECK(ConvertCSSValueToStyle<DynamicRangeLimit>(*State,
                                                    *css::PrimitiveValue::Create(css::ValueId::Constrained))
          == DynamicRangeLimit {css::keywords::Constrained {}});
    CHECK(Created(DynamicRangeLimit {css::keywords::NoLimit {}}) == "no-limit");
    CHECK_FALSE(IsInvalid());

    CHECK(ConvertCSSValueToStyle<DynamicRangeLimit>(*State, *css::PrimitiveValue::Create(css::ValueId::Auto))
          == DynamicRangeLimit {css::keywords::NoLimit {}});
    CHECK(IsInvalid());
  }

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: opacity", "[Style][CSSValueConversion]")
  {
    CHECK(Convert<Opacity>(PropertyId::Opacity, u8"0.25") == Opacity {0.25f});
    CHECK(Convert<Opacity>(PropertyId::Opacity, u8"50%") == Opacity {0.5f});
    CHECK(Convert<Opacity>(PropertyId::Opacity, u8"2") == Opacity {1.f});
    CHECK(Convert<Opacity>(PropertyId::Opacity, u8"-1") == Opacity {0.f});
    CHECK_FALSE(IsInvalid());

    CHECK(ConvertCSSValueToStyle<Opacity>(*State, *css::ValueList::CreateSpaceSeparated()) == Opacity {1.f});
    CHECK(IsInvalid());
  }
}
