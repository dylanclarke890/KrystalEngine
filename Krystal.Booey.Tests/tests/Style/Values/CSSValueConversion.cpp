#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Ratio.hpp"

namespace krys::boo::style::tests
{
  struct ConversionTestSize
      : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::Auto, css::keywords::MinContent>
  {
    using Base::Base;
  };
}

namespace krys::boo::css
{
  template <>
  constexpr bool TreatAsVariantLike<style::tests::ConversionTestSize> = true;
}

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using css::UnitType;
  using css::ValueId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: strings and identifiers",
                   "[Style][CSSValueConversion]")
  {
    SECTION("Strings")
    {
      CHECK(ConvertCSSValueToStyle<css::CSSOMString>(*State,
                                                     *css::PrimitiveValue::Create(css::CSSOMString {u8"abc"}))
            == u8"abc");
      CHECK(ConvertCSSValueToStyle<css::CSSOMStringAtom>(
              *State, *css::PrimitiveValue::Create(css::CSSOMString {u8"abc"}))
            == css::CSSOMStringAtom {u8"abc"});
      CHECK_FALSE(IsInvalid());
    }

    SECTION("A string from a non-string is invalid at computed-value time")
    {
      CHECK(ConvertCSSValueToStyle<css::CSSOMString>(*State, *css::PrimitiveValue::Create(ValueId::Auto))
              .empty());
      CHECK(IsInvalid());
    }

    SECTION("A string from a non-primitive value is invalid at computed-value time")
    {
      CHECK(
        ConvertCSSValueToStyle<css::CSSOMString>(*State, *css::ValueList::CreateSpaceSeparated()).empty());
      CHECK(IsInvalid());
    }

    SECTION("Custom identifiers")
    {
      CHECK(ConvertCSSValueToStyle<css::CustomIdentifier>(
              *State, *css::PrimitiveValue::CreateCustomIdent(css::CSSOMString {u8"foo"}))
              .Value
            == css::CSSOMStringAtom {u8"foo"});
      CHECK_FALSE(IsInvalid());

      CHECK(ConvertCSSValueToStyle<css::CustomIdentifier>(*State, *css::PrimitiveValue::Create(ValueId::Auto))
              .Value
            == css::CSSOMStringAtom::Empty());
      CHECK(IsInvalid());
    }

    SECTION("Property identifiers")
    {
      CHECK(ConvertCSSValueToStyle<css::PropertyIdentifier>(*State,
                                                            *css::PrimitiveValue::Create(PropertyId::Color))
              .Value
            == PropertyId::Color);
      CHECK_FALSE(IsInvalid());

      CHECK(
        ConvertCSSValueToStyle<css::PropertyIdentifier>(*State, *css::PrimitiveValue::Create(ValueId::Auto))
          .Value
        == PropertyId::Invalid);
      CHECK(IsInvalid());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: keywords", "[Style][CSSValueConversion]")
  {
    SECTION("Enums convert from their keyword")
    {
      CHECK(Convert<Visibility>(PropertyId::Visibility, u8"hidden") == Visibility::Hidden);
      CHECK(Convert<BorderStyle>(PropertyId::BorderTopStyle, u8"dashed") == BorderStyle::Dashed);
      CHECK(Convert<DisplayType>(PropertyId::Display, u8"inline-block") == DisplayType::InlineBlock);
    }

    SECTION("Creation and serialisation, with and without a value representation")
    {
      CHECK(Created(Visibility::Collapse) == "collapse");
      CHECK(Serialised(Visibility::Collapse) == "collapse");
      CHECK(Created(BorderStyle::Groove) == "groove");
      CHECK(Serialised(BorderStyle::Groove) == "groove");
      CHECK(Created(StyleAppearance::MenulistButton) == "menulist-button");
    }

    SECTION("Lists of keywords")
    {
      auto list = css::ValueList::CreateSpaceSeparated(css::PrimitiveValue::Create(ValueId::Hidden),
                                                       css::PrimitiveValue::Create(ValueId::Visible));
      auto values = ConvertCSSValueToStyle<css::SpaceSeparatedFixedList<Visibility>>(*State, *list);
      REQUIRE(values.size() == 2);
      CHECK(values[0] == Visibility::Hidden);
      CHECK(values[1] == Visibility::Visible);

      // A single value is a list of one.
      auto single = ConvertCSSValueToStyle<css::SpaceSeparatedFixedList<Visibility>>(
        *State, *css::PrimitiveValue::Create(ValueId::Collapse));
      REQUIRE(single.size() == 1);
      CHECK(single[0] == Visibility::Collapse);

      // A comma-separated list isn't a space-separated one.
      auto commaList = css::ValueList::CreateCommaSeparated(css::PrimitiveValue::Create(ValueId::Hidden));
      auto commas = ConvertCSSValueToStyle<css::CommaSeparatedFixedList<Visibility>>(*State, *commaList);
      REQUIRE(commas.size() == 1);
      CHECK(commas[0] == Visibility::Hidden);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: numbers", "[Style][CSSValueConversion]")
  {
    SECTION("Integers")
    {
      CHECK(Convert<Integer<>>(PropertyId::ZIndex, u8"7").Value == 7);
      CHECK(Convert<Integer<>>(PropertyId::Order, u8"-2").Value == -2);
      CHECK(Convert<Integer<>>(PropertyId::ZIndex, u8"calc(2 * 3)").Value == 6);
    }

    SECTION("Numbers, clamped to their range")
    {
      CHECK(Convert<Number<>>(PropertyId::LineHeight, u8"1.5").Value == 1.5);
      CHECK(ConvertCSSValueToStyle<Number<css::NonNegative>>(*State, *css::PrimitiveValue::Create(-3)).Value
            == 0);
    }

    SECTION("Percentages")
    {
      CHECK(
        ConvertCSSValueToStyle<Percentage<>>(*State, *css::PrimitiveValue::Create(25, UnitType::Percentage))
          .Value
        == 25);
    }

    SECTION("Angles")
    {
      CHECK(ConvertCSSValueToStyle<Angle<>>(*State, *css::PrimitiveValue::Create(0.25, UnitType::turn)).Value
            == 90);
    }

    SECTION("Lengths resolve against the style's font")
    {
      CHECK(Convert<Length<>>(PropertyId::OutlineOffset, u8"2em").UnresolvedValue() == 32);
      CHECK(Convert<Length<>>(PropertyId::OutlineOffset, u8"1in").UnresolvedValue() == 96);
    }

    SECTION("Viewport units mark the style")
    {
      CHECK_FALSE(Style.UsesViewportUnits());
      std::ignore = Convert<Length<>>(PropertyId::OutlineOffset, u8"10vw");
      CHECK(Style.UsesViewportUnits());
    }

    SECTION("Times, resolutions and flex")
    {
      CHECK(ConvertCSSValueToStyle<Time<>>(*State, *css::PrimitiveValue::Create(500, UnitType::ms)).Value
            == 0.5);
      CHECK(ConvertCSSValueToStyle<Resolution<>>(*State, *css::PrimitiveValue::Create(2, UnitType::x)).Value
            == 2);
      CHECK(ConvertCSSValueToStyle<Flex<>>(*State, *css::PrimitiveValue::Create(3, UnitType::fr)).Value == 3);
    }

    SECTION("Length-percentages")
    {
      auto length = Convert<LengthPercentage<>>(PropertyId::TextIndent, u8"3px");
      REQUIRE(length.HoldsAlternative<LengthPercentage<>::Dimension>());
      CHECK(Serialised(length) == "3px");

      auto percentage = Convert<LengthPercentage<>>(PropertyId::TextIndent, u8"25%");
      REQUIRE(percentage.HoldsAlternative<LengthPercentage<>::Percentage>());
      CHECK(Serialised(percentage) == "25%");

      auto calc = Convert<LengthPercentage<>>(PropertyId::TextIndent, u8"calc(10px + 5%)");
      REQUIRE(calc.HoldsAlternative<LengthPercentage<>::Calc>());
      CHECK(Serialised(calc) == "calc(5% + 10px)");
    }

    SECTION("Number-or-percentages")
    {
      auto number = ConvertCSSValueToStyle<NumberOrPercentage<>>(*State, *css::PrimitiveValue::Create(0.5));
      CHECK(Serialised(number) == "0.5");
      auto percentage = ConvertCSSValueToStyle<NumberOrPercentage<>>(
        *State, *css::PrimitiveValue::Create(50, UnitType::Percentage));
      CHECK(Serialised(percentage) == "50%");
      auto resolved = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(
        *State, *css::PrimitiveValue::Create(50, UnitType::Percentage));
      CHECK(Serialised(resolved) == "0.5");
    }

    SECTION("A non-primitive value is invalid at computed-value time")
    {
      CHECK(
        ConvertCSSValueToStyle<Length<>>(*State, *css::ValueList::CreateSpaceSeparated()).UnresolvedValue()
        == 0);
      CHECK(IsInvalid());
    }

    SECTION("Creation")
    {
      CHECK(Created(Length<> {32}) == "32px");
      CHECK(Created(Number<> {1.5}) == "1.5");
      CHECK(Created(Percentage<> {20}) == "20%");
      CHECK(Created(Ratio {16, 9}) == "16 / 9");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "CSSValueConversion: length wrappers", "[Style][CSSValueConversion]")
  {
    SECTION("Keywords")
    {
      CHECK(Convert<ConversionTestSize>(PropertyId::Width, u8"auto").HoldsAlternative<css::keywords::Auto>());
      CHECK(Convert<ConversionTestSize>(PropertyId::Width, u8"min-content")
              .HoldsAlternative<css::keywords::MinContent>());
      CHECK(ConvertCSSValueToStyle<ConversionTestSize>(
              *State, *css::PrimitiveValue::Create(ValueId::WebkitMinContent))
              .HoldsAlternative<css::keywords::MinContent>());
      CHECK_FALSE(IsInvalid());
    }

    SECTION("Unsupported keywords are invalid at computed-value time")
    {
      CHECK(Convert<ConversionTestSize>(PropertyId::Width, u8"max-content") == ConversionTestSize {0_css_px});
      CHECK(IsInvalid());
    }

    SECTION("Lengths, percentages and calc")
    {
      auto fixed = Convert<ConversionTestSize>(PropertyId::Width, u8"2em");
      REQUIRE(fixed.IsFixed());
      CHECK(fixed.TryFixed()->UnresolvedValue() == 32);
      CHECK(Convert<ConversionTestSize>(PropertyId::Width, u8"50%").IsPercent());
      CHECK(Convert<ConversionTestSize>(PropertyId::Width, u8"calc(50% - 10px)").IsCalculated());
    }

    SECTION("Deprecated conversion without a renderer")
    {
      RefPtr<dom::Element> element;
      auto px =
        DeprecatedCSSValueConversion<ConversionTestSize> {}(element, *Parse(PropertyId::Width, u8"10px"));
      REQUIRE(px);
      CHECK(px->TryFixed()->UnresolvedValue() == 10);
      CHECK(DeprecatedCSSValueConversion<ConversionTestSize> {}(element, *Parse(PropertyId::Width, u8"50%"))
              ->IsPercent());
      CHECK_FALSE(
        DeprecatedCSSValueConversion<ConversionTestSize> {}(element, *Parse(PropertyId::Width, u8"2em")));
      CHECK_FALSE(DeprecatedCSSValueConversion<ConversionTestSize> {}(
        element, *Parse(PropertyId::Width, u8"calc(1px + 2%)")));
      CHECK(DeprecatedCSSValueConversion<ConversionTestSize> {}(element, *Parse(PropertyId::Width, u8"auto"))
              ->HoldsAlternative<css::keywords::Auto>());
    }
  }
}
