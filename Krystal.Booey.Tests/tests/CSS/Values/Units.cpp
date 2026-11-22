#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

#include <numbers>

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

  TEST_CASE("CategoryForUnitType", "[CSS][Units]")
  {
    CHECK(CategoryForUnitType(UnitType::Number) == Category::Number);
    CHECK(CategoryForUnitType(UnitType::Integer) == Category::Integer);
    CHECK(CategoryForUnitType(UnitType::Percentage) == Category::Percentage);
    for (auto unit : {UnitType::px, UnitType::Q, UnitType::em, UnitType::QuirkyEm, UnitType::rem,
                      UnitType::vw, UnitType::dvi, UnitType::cqw})
    {
      INFO(static_cast<int>(unit));
      CHECK(CategoryForUnitType(unit) == Category::Length);
    }
    CHECK(CategoryForUnitType(UnitType::turn) == Category::Angle);
    CHECK(CategoryForUnitType(UnitType::ms) == Category::Time);
    CHECK(CategoryForUnitType(UnitType::khz) == Category::Frequency);
    CHECK(CategoryForUnitType(UnitType::x) == Category::Resolution);
    CHECK(CategoryForUnitType(UnitType::fr) == Category::Flex);
    CHECK_FALSE(CategoryForUnitType(UnitType::Unknown));
  }

  TEST_CASE("CanonicalUnitTypeForCategory", "[CSS][Units]")
  {
    CHECK(CanonicalUnitTypeForCategory(Category::Integer) == UnitType::Number);
    CHECK(CanonicalUnitTypeForCategory(Category::Number) == UnitType::Number);
    CHECK(CanonicalUnitTypeForCategory(Category::Percentage) == UnitType::Percentage);
    CHECK(CanonicalUnitTypeForCategory(Category::Length) == UnitType::px);
    CHECK(CanonicalUnitTypeForCategory(Category::Angle) == UnitType::deg);
    CHECK(CanonicalUnitTypeForCategory(Category::Time) == UnitType::s);
    CHECK(CanonicalUnitTypeForCategory(Category::Frequency) == UnitType::hz);
    CHECK(CanonicalUnitTypeForCategory(Category::Resolution) == UnitType::dppx);
    CHECK(CanonicalUnitTypeForCategory(Category::Flex) == UnitType::fr);
    CHECK(CanonicalUnitTypeForCategory(Category::LengthPercentage) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForCategory(Category::AnglePercentage) == UnitType::Unknown);
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
    CHECK(CanonicalUnitTypeForUnitType(UnitType::vw) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::cqw) == UnitType::Unknown);
    CHECK(CanonicalUnitTypeForUnitType(UnitType::Unknown) == UnitType::Unknown);
  }

  TEST_CASE("ConversionToCanonicalUnitsScaleFactor", "[CSS][Units]")
  {
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::px) == 1.0);
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::in) == 96.0);
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::cm) == Catch::Approx(96.0 / 2.54));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::mm) == Catch::Approx(96.0 / 25.4));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::Q) == Catch::Approx(96.0 / 101.6));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::pt) == Catch::Approx(96.0 / 72.0));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::pc) == Catch::Approx(16.0));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::turn) == 360.0);
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::grad) == Catch::Approx(0.9));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::rad) == Catch::Approx(57.29577951));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::ms) == Catch::Approx(0.001));
    CHECK(ConversionToCanonicalUnitsScaleFactor(UnitType::khz) == 1000.0);
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::dpi) == Catch::Approx(1.0 / 96.0));
    CHECK(*ConversionToCanonicalUnitsScaleFactor(UnitType::dpcm) == Catch::Approx(2.54 / 96.0));
    // Units without a fixed factor have none.
    CHECK_FALSE(ConversionToCanonicalUnitsScaleFactor(UnitType::em));
    CHECK_FALSE(ConversionToCanonicalUnitsScaleFactor(UnitType::cqw));
    CHECK_FALSE(ConversionToCanonicalUnitsScaleFactor(UnitType::Number));
    CHECK_FALSE(ConversionToCanonicalUnitsScaleFactor(UnitType::fr));
    CHECK_FALSE(ConversionToCanonicalUnitsScaleFactor(UnitType::Unknown));
  }

  TEST_CASE("ConversionToCanonicalUnitRequiresConversionData", "[CSS][Units]")
  {
    // Relative lengths, which have no fixed ratio to px.
    for (auto unit : {UnitType::em, UnitType::rem, UnitType::lh, UnitType::ch, UnitType::vw, UnitType::svh,
                      UnitType::lvmin, UnitType::dvmax, UnitType::cqw, UnitType::cqmax, UnitType::QuirkyEm})
    {
      INFO(static_cast<int>(unit));
      CHECK(ConversionToCanonicalUnitRequiresConversionData(unit));
    }

    for (auto unit :
         {UnitType::Number, UnitType::Integer, UnitType::Percentage, UnitType::px, UnitType::cm, UnitType::mm,
          UnitType::Q, UnitType::in, UnitType::pt, UnitType::pc, UnitType::deg, UnitType::turn, UnitType::s,
          UnitType::ms, UnitType::hz, UnitType::dppx, UnitType::dpi, UnitType::fr, UnitType::Unknown})
    {
      INFO(static_cast<int>(unit));
      CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(unit));
    }
  }

  TEST_CASE("UnitTypeString", "[CSS][Units]")
  {
    CHECK(ToNarrow(UnitTypeString(UnitType::px)) == "px");
    CHECK(ToNarrow(UnitTypeString(UnitType::Percentage)) == "%");
    CHECK(ToNarrow(UnitTypeString(UnitType::Q)) == "q");
    CHECK(ToNarrow(UnitTypeString(UnitType::dvmax)) == "dvmax");
    CHECK(ToNarrow(UnitTypeString(UnitType::khz)) == "khz");
    CHECK(ToNarrow(UnitTypeString(UnitType::x)) == "x");

    // Units that don't serialise with a suffix.
    CHECK(ToNarrow(UnitTypeString(UnitType::QuirkyEm)) == "em");

    for (auto unit : {UnitType::Number, UnitType::Integer, UnitType::Unknown})
    {
      INFO(static_cast<int>(unit));
      CHECK(UnitTypeString(unit).empty());
    }
  }

  TEST_CASE("Units: every unit's name parses back to it", "[CSS][Units]")
  {
    for (size_t i = 0uz; i < (ToUnderlying(MaxEnumValue(UnitType {})) + 1uz); ++i)
    {
      auto unit = static_cast<UnitType>(i);
      auto name = UnitTypeString(unit);
      INFO(i);
      // Every length, angle, time, frequency, resolution and flex unit has a name, but QuirkyEm.
      auto category = CategoryForUnitType(unit);
      if (unit != UnitType::QuirkyEm && category && category != Category::Number
          && category != Category::Integer)
      {
        CHECK_FALSE(name.empty());
      }
      if (!name.empty() && unit != UnitType::Percentage && unit != UnitType::QuirkyEm)
      {
        CHECK(ParseUnitType(name) == unit);
      }
    }
  }

  TEST_CASE("IsLength", "[CSS][Units]")
  {
    for (auto unit :
         {UnitType::px, UnitType::cm, UnitType::mm, UnitType::in, UnitType::pt, UnitType::pc, UnitType::Q,
          UnitType::em, UnitType::ex, UnitType::rem, UnitType::lh, UnitType::cap, UnitType::ch, UnitType::ic,
          UnitType::QuirkyEm, UnitType::vw, UnitType::dvi, UnitType::cqw, UnitType::cqmax})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsLength(unit));
    }

    for (auto unit : {UnitType::Number, UnitType::Integer, UnitType::Percentage, UnitType::deg, UnitType::s,
                      UnitType::hz, UnitType::dppx, UnitType::fr, UnitType::Unknown})
    {
      INFO(static_cast<int>(unit));
      CHECK_FALSE(IsLength(unit));
    }
  }

  TEST_CASE("IsFontRelativeLength", "[CSS][Units]")
  {
    for (auto unit : {UnitType::em, UnitType::ex, UnitType::lh, UnitType::cap, UnitType::ch, UnitType::ic,
                      UnitType::QuirkyEm, UnitType::rem, UnitType::rex, UnitType::rlh, UnitType::rcap,
                      UnitType::rch, UnitType::ric})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsFontRelativeLength(unit));
    }

    CHECK_FALSE(IsFontRelativeLength(UnitType::px));
    CHECK_FALSE(IsFontRelativeLength(UnitType::vw));
    CHECK_FALSE(IsFontRelativeLength(UnitType::Percentage));
  }

  TEST_CASE("IsRootFontRelativeLength", "[CSS][Units]")
  {
    for (auto unit :
         {UnitType::rem, UnitType::rex, UnitType::rlh, UnitType::rcap, UnitType::rch, UnitType::ric})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsRootFontRelativeLength(unit));
    }

    CHECK_FALSE(IsRootFontRelativeLength(UnitType::em));
    CHECK_FALSE(IsRootFontRelativeLength(UnitType::lh));
  }

  TEST_CASE("IsFontIndependentLength", "[CSS][Units]")
  {
    for (auto unit :
         {UnitType::px, UnitType::cm, UnitType::mm, UnitType::Q, UnitType::in, UnitType::pt, UnitType::pc})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsFontIndependentLength(unit));
    }

    CHECK_FALSE(IsFontIndependentLength(UnitType::em));
  }

  TEST_CASE("IsViewportPercentageLength", "[CSS][Units]")
  {
    for (auto unit :
         {UnitType::vw, UnitType::vh, UnitType::vmin, UnitType::vmax, UnitType::vb, UnitType::vi,
          UnitType::svw, UnitType::svi, UnitType::lvw, UnitType::lvi, UnitType::dvw, UnitType::dvi})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsViewportPercentageLength(unit));
    }

    CHECK_FALSE(IsViewportPercentageLength(UnitType::cqw));
    CHECK_FALSE(IsViewportPercentageLength(UnitType::Percentage));
  }

  TEST_CASE("IsContainerPercentageLength", "[CSS][Units]")
  {
    for (auto unit :
         {UnitType::cqw, UnitType::cqh, UnitType::cqi, UnitType::cqb, UnitType::cqmin, UnitType::cqmax})
    {
      INFO(static_cast<int>(unit));
      CHECK(IsContainerPercentageLength(unit));
    }

    CHECK_FALSE(IsContainerPercentageLength(UnitType::vw));
  }

  TEST_CASE("ConvertUnit", "[CSS][Units]")
  {
    constexpr double pi = std::numbers::pi;

    CHECK(ConvertUnit(90.0, UnitType::deg, UnitType::deg) == 90.0);
    CHECK(*ConvertUnit(pi, UnitType::rad, UnitType::deg) == Catch::Approx(180.0));
    CHECK(*ConvertUnit(200.0, UnitType::grad, UnitType::deg) == Catch::Approx(180.0));
    CHECK(*ConvertUnit(0.5, UnitType::turn, UnitType::deg) == Catch::Approx(180.0));
    CHECK(*ConvertUnit(180.0, UnitType::deg, UnitType::rad) == Catch::Approx(pi));
    CHECK(*ConvertUnit(1.0, UnitType::turn, UnitType::rad) == Catch::Approx(2.0 * pi));
    CHECK(*ConvertUnit(90.0, UnitType::deg, UnitType::grad) == Catch::Approx(100.0));
    CHECK(*ConvertUnit(100.0, UnitType::grad, UnitType::turn) == Catch::Approx(0.25));

    CHECK(*ConvertUnit(250.0, UnitType::ms, UnitType::s) == Catch::Approx(0.25));
    CHECK(*ConvertUnit(1.5, UnitType::s, UnitType::ms) == Catch::Approx(1500.0));
    CHECK(*ConvertUnit(2.0, UnitType::khz, UnitType::hz) == Catch::Approx(2000.0));
    CHECK(*ConvertUnit(500.0, UnitType::hz, UnitType::khz) == Catch::Approx(0.5));

    CHECK(*ConvertUnit(2.0, UnitType::x, UnitType::dppx) == Catch::Approx(2.0));
    CHECK(*ConvertUnit(192.0, UnitType::dpi, UnitType::dppx) == Catch::Approx(2.0));
    CHECK(*ConvertUnit(1.0, UnitType::dpcm, UnitType::dpi) == Catch::Approx(2.54));
    CHECK(*ConvertUnit(1.0, UnitType::in, UnitType::cm) == Catch::Approx(2.54));

    // Different quantities, and units without a fixed factor, don't convert.
    CHECK_FALSE(ConvertUnit(1.0, UnitType::deg, UnitType::s));
    CHECK_FALSE(ConvertUnit(1.0, UnitType::em, UnitType::px));
    CHECK_FALSE(ConvertUnit(1.0, UnitType::Number, UnitType::Percentage));
  }

  TEST_CASE("ConvertUnitOrNumber", "[CSS][Units]")
  {
    CHECK(*ConvertUnitOrNumber(1.0, UnitType::in, UnitType::px) == Catch::Approx(96.0));
    // A number is in the canonical unit of the other side.
    CHECK(*ConvertUnitOrNumber(1.0, UnitType::in, UnitType::Number) == Catch::Approx(96.0));
    CHECK(*ConvertUnitOrNumber(96.0, UnitType::Number, UnitType::in) == Catch::Approx(1.0));
    CHECK(*ConvertUnitOrNumber(0.5, UnitType::turn, UnitType::Number) == Catch::Approx(180.0));
    CHECK(ConvertUnitOrNumber(3.0, UnitType::Integer, UnitType::Number) == 3.0);
    CHECK(ConvertUnitOrNumber(2.0, UnitType::fr, UnitType::Number) == 2.0);

    // Numbers and percentages don't convert, nor do relative lengths and unrelated quantities.
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::Number, UnitType::Percentage));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::Percentage, UnitType::Number));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::em, UnitType::Number));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::Number, UnitType::vw));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::em, UnitType::rem));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::deg, UnitType::px));
    CHECK_FALSE(ConvertUnitOrNumber(1.0, UnitType::Unknown, UnitType::px));
  }
}
