#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("ParseUnitType", "[CSS][Units]")
  {
    CHECK(ParseUnitType(u8"px") == UnitType::px);
    CHECK(ParseUnitType(u8"PX") == UnitType::px);
    CHECK(ParseUnitType(u8"Em") == UnitType::em);
    CHECK(ParseUnitType(u8"q") == UnitType::Q);
    CHECK(ParseUnitType(u8"Q") == UnitType::Q);
    CHECK(ParseUnitType(u8"s") == UnitType::s);
    CHECK(ParseUnitType(u8"x") == UnitType::x);
    CHECK(ParseUnitType(u8"kHz") == UnitType::khz);
    CHECK(ParseUnitType(u8"dvmax") == UnitType::dvmax);
    CHECK(ParseUnitType(u8"cqmin") == UnitType::cqmin);
    CHECK(ParseUnitType(u8"dppx") == UnitType::dppx);
    CHECK(ParseUnitType(u8"turn") == UnitType::turn);
    CHECK(ParseUnitType(u8"__qem") == UnitType::QuirkyEm);

    CHECK(ParseUnitType(u8"") == UnitType::Unknown);
    CHECK(ParseUnitType(u8"foo") == UnitType::Unknown);
    CHECK(ParseUnitType(u8"pxx") == UnitType::Unknown);
    CHECK(ParseUnitType(u8"%") == UnitType::Unknown);
    CHECK(ParseUnitType(u8"number") == UnitType::Unknown);

    static_assert(ParseUnitType(u8"rem") == UnitType::rem);
  }

  TEST_CASE("UnitCategoryForType", "[CSS][Units]")
  {
    CHECK(UnitCategoryForType(UnitType::Number) == UnitCategory::Number);
    CHECK(UnitCategoryForType(UnitType::Integer) == UnitCategory::Number);
    CHECK(UnitCategoryForType(UnitType::Percentage) == UnitCategory::Percent);
    CHECK(UnitCategoryForType(UnitType::px) == UnitCategory::AbsoluteLength);
    CHECK(UnitCategoryForType(UnitType::cm) == UnitCategory::AbsoluteLength);
    CHECK(UnitCategoryForType(UnitType::Q) == UnitCategory::AbsoluteLength);
    CHECK(UnitCategoryForType(UnitType::em) == UnitCategory::FontRelativeLength);
    CHECK(UnitCategoryForType(UnitType::rem) == UnitCategory::FontRelativeLength);
    CHECK(UnitCategoryForType(UnitType::vw) == UnitCategory::ViewportPercentageLength);
    CHECK(UnitCategoryForType(UnitType::dvi) == UnitCategory::ViewportPercentageLength);
    CHECK(UnitCategoryForType(UnitType::deg) == UnitCategory::Angle);
    CHECK(UnitCategoryForType(UnitType::turn) == UnitCategory::Angle);
    CHECK(UnitCategoryForType(UnitType::ms) == UnitCategory::Time);
    CHECK(UnitCategoryForType(UnitType::khz) == UnitCategory::Frequency);
    CHECK(UnitCategoryForType(UnitType::dpi) == UnitCategory::Resolution);
    CHECK(UnitCategoryForType(UnitType::x) == UnitCategory::Resolution);
    CHECK(UnitCategoryForType(UnitType::fr) == UnitCategory::Flex);
    CHECK(UnitCategoryForType(UnitType::cqw) == UnitCategory::Other);
    CHECK(UnitCategoryForType(UnitType::Calc) == UnitCategory::Other);
    CHECK(UnitCategoryForType(UnitType::ValueId) == UnitCategory::Other);
    CHECK(UnitCategoryForType(UnitType::QuirkyEm) == UnitCategory::Other);
  }

  TEST_CASE("UnitCategoryForType(every unit)", "[CSS][Units]")
  {
    // Every unit has a category (none falls through to unreachable).
    for (size_t i = 0uz; i < EnumTraits<UnitType>::DistinctValues; ++i)
    {
      INFO(i);
      auto category = UnitCategoryForType(static_cast<UnitType>(i));
      CHECK(static_cast<size_t>(category) < EnumTraits<UnitCategory>::DistinctValues);
    }
  }

  TEST_CASE("CanonicalUnitTypeForCategory", "[CSS][Units]")
  {
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Number) == UnitType::Number);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Percent) == UnitType::Percentage);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::AbsoluteLength) == UnitType::px);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Angle) == UnitType::deg);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Time) == UnitType::s);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Frequency) == UnitType::hz);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Resolution) == UnitType::dppx);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Flex) == UnitType::fr);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::FontRelativeLength) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::ViewportPercentageLength) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForCategory(UnitCategory::Other) == UnitType::Unknown);
  }

  TEST_CASE("CanonicalUnitTypeForUnitType", "[CSS][Units]")
  {
    CHECK(CanonicalUnitTypeForUnitType(UnitType::cm) == UnitType::px);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::px) == UnitType::px);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::turn) == UnitType::deg);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::ms) == UnitType::s);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::khz) == UnitType::hz);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::dpi) == UnitType::dppx);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::Integer) == UnitType::Number);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::em) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::String) == UnitType::Unknown);
  }

  TEST_CASE("ConversionToCanonicalUnitsScaleFactor", "[CSS][Units]")
  {
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::px) == 1.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::in) == 96.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::cm) == Catch::Approx(96.0 / 2.54));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::mm) == Catch::Approx(96.0 / 25.4));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::Q) == Catch::Approx(96.0 / 101.6));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::pt) == Catch::Approx(96.0 / 72.0));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::pc) == Catch::Approx(16.0));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::turn) == 360.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::grad) == Catch::Approx(0.9));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::rad) == Catch::Approx(57.29577951));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::ms) == Catch::Approx(0.001));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::khz) == 1000.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::dpi) == Catch::Approx(1.0 / 96.0));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::dpcm) == Catch::Approx(2.54 / 96.0));
    // Everything else is treated as already canonical.
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::em) == 1.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::Number) == 1.0);
  }

  TEST_CASE("ConversionToCanonicalUnitRequiresConversionData", "[CSS][Units]")
  {
    // Absolute lengths other than px, and font/viewport/container relative lengths.
    for (auto unit : {UnitType::cm, UnitType::mm, UnitType::Q, UnitType::in, UnitType::pt, UnitType::pc,
                      UnitType::em, UnitType::rem, UnitType::lh, UnitType::ch, UnitType::vw, UnitType::svh,
                      UnitType::lvmin, UnitType::dvmax, UnitType::cqw, UnitType::cqmax})
    {
      INFO(static_cast<int>(unit));
      CHECK(ConversionToCanonicalUnitRequiresConversionData(unit));
    }

    for (auto unit : {UnitType::Number, UnitType::Integer, UnitType::Percentage, UnitType::px, UnitType::deg,
                      UnitType::turn, UnitType::s, UnitType::ms, UnitType::hz, UnitType::dppx, UnitType::dpi,
                      UnitType::fr, UnitType::Calc, UnitType::QuirkyEm, UnitType::ValueId})
    {
      INFO(static_cast<int>(unit));
      CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(unit));
    }
  }
}
