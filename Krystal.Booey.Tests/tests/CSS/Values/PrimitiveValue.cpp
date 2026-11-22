#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include <numbers>

namespace krys::boo::css::tests
{
  TEST_CASE("RoundForImpreciseConversion", "[CSS][PrimitiveValue]")
  {
    CHECK(RoundForImpreciseConversion<int32>(44.99998) == 45);
    CHECK(RoundForImpreciseConversion<int32>(44.5) == 44);
    CHECK(RoundForImpreciseConversion<int32>(-44.99998) == -45);
    CHECK(RoundForImpreciseConversion<uint16>(1e9) == 0);
    CHECK(RoundForImpreciseConversion<int16>(-1e9) == 0);

    CHECK(RoundForImpreciseConversion<float>(44.995) == 45.0f);
    CHECK(RoundForImpreciseConversion<float>(44.5) == 44.5f);
    CHECK(RoundForImpreciseConversion<float>(-44.995) == -45.0f);
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValue::ResolveAsLength", "[CSS][PrimitiveValue]")
  {
    auto conversionData = ToLengthConversionData {};
    auto pixels = PrimitiveValue::Create(12.5, UnitType::px);
    auto inches = PrimitiveValue::Create(1.0, UnitType::in);

    CHECK(pixels->ResolveAsLength(conversionData) == 12.5);
    CHECK(inches->ResolveAsLength(conversionData) == 96.0);
    CHECK(inches->ResolveAsLength<float>(conversionData) == 96.0f);
    CHECK(inches->ResolveAsLength<LayoutUnit>(conversionData) == LayoutUnit(96.0));
    CHECK(pixels->ResolveAsLength<LayoutUnit>(conversionData) == LayoutUnit(12.5));

    // Integer results round imprecise conversions.
    CHECK(pixels->ResolveAsLength<int32>(conversionData) == 12);
    CHECK(PrimitiveValue::Create(44.99998, UnitType::px)->ResolveAsLength<int32>(conversionData) == 45);

    // Absolute lengths are scaled by zoom.
    CHECK(inches->ResolveAsLength(conversionData.CopyWithAdjustedZoom(2.0f)) == 192.0);
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValue: resolving without an element", "[CSS][PrimitiveValue]")
  {
    const auto &conversionData = ToLengthConversionData::WithoutElement();

    // Font-relative units resolve against the initial font, 16px.
    CHECK(PrimitiveValue::Create(2.0, UnitType::em)->ResolveAsLength(conversionData) == 32.0);
    CHECK(PrimitiveValue::Create(2.0, UnitType::rem)->ResolveAsLength(conversionData) == 32.0);
    CHECK(PrimitiveValue::Create(1.0, UnitType::ex)->ResolveAsLength(conversionData) > 0.0);
    CHECK(PrimitiveValue::Create(1.0, UnitType::ch)->ResolveAsLength(conversionData) > 0.0);
    // There's no viewport.
    CHECK(PrimitiveValue::Create(50.0, UnitType::vw)->ResolveAsLength(conversionData) == 0.0);
    // Other values resolve as with any context.
    CHECK(PrimitiveValue::Create(3.0, UnitType::Integer)->ResolveAsInteger(conversionData) == 3);
    CHECK(PrimitiveValue::Create(25.0, UnitType::Percentage)->ResolveAsPercentage(conversionData) == 25.0);
  }
}
