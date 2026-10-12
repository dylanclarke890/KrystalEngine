#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  using Catch::Approx;

  TEST_CASE("PrimitiveUnits(concepts)", "[CSS][Primitives][Units]")
  {
    STATIC_CHECK(UnitEnum<IntegerUnit>);
    STATIC_CHECK(UnitEnum<LengthUnit>);
    // Regression: the traits used lowercase names, so AnglePercentageUnit wasn't a UnitEnum.
    STATIC_CHECK(UnitEnum<AnglePercentageUnit>);
    STATIC_CHECK(UnitEnum<LengthPercentageUnit>);

    STATIC_CHECK(SingleValueUnitEnum<NumberUnit>);
    STATIC_CHECK(SingleValueUnitEnum<FlexUnit>);
    STATIC_CHECK_FALSE(SingleValueUnitEnum<LengthUnit>);

    STATIC_CHECK(CompositeUnitEnum<AnglePercentageUnit>);
    STATIC_CHECK_FALSE(CompositeUnitEnum<AngleUnit>);

    STATIC_CHECK(NestedUnitEnumOf<AngleUnit, AnglePercentageUnit>);
    STATIC_CHECK(NestedUnitEnumOf<PercentageUnit, LengthPercentageUnit>);
    STATIC_CHECK_FALSE(NestedUnitEnumOf<AngleUnit, LengthPercentageUnit>);
  }

  TEST_CASE("PrimitiveUnits(traits)", "[CSS][Primitives][Units]")
  {
    STATIC_CHECK(UnitTraits<IntegerUnit>::Category == Category::Integer);
    STATIC_CHECK(UnitTraits<LengthUnit>::Category == Category::Length);
    STATIC_CHECK(UnitTraits<LengthPercentageUnit>::Category == Category::LengthPercentage);
    STATIC_CHECK(UnitTraits<AnglePercentageUnit>::Category == Category::AnglePercentage);

    STATIC_CHECK(UnitTraits<LengthUnit>::Count == 50);
    STATIC_CHECK(UnitTraits<LengthPercentageUnit>::Count == 51);
    STATIC_CHECK(UnitTraits<AnglePercentageUnit>::Count == 5);

    STATIC_CHECK(UnitTraits<LengthUnit>::Canonical == LengthUnit::Px);
    STATIC_CHECK(UnitTraits<AngleUnit>::Canonical == AngleUnit::Deg);
    STATIC_CHECK(UnitTraits<ResolutionUnit>::Canonical == ResolutionUnit::Dppx);

    STATIC_CHECK(UnitTraits<ResolutionUnit>::IsValidRangeForCategory(NonNegative));
    STATIC_CHECK_FALSE(UnitTraits<ResolutionUnit>::IsValidRangeForCategory(All));
  }

  TEST_CASE("PrimitiveUnits(UnitType round trips)", "[CSS][Primitives][Units]")
  {
    // Every UnitType that maps to a unit maps back to itself.
    for (size_t i = 0uz; i < EnumTraits<UnitType>::DistinctValues; ++i)
    {
      auto unitType = static_cast<UnitType>(i);
      INFO(i);

      if (auto unit = ToLengthUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
        CHECK(ToLengthPercentageUnit(unitType).has_value());
      }
      if (auto unit = ToLengthPercentageUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
      if (auto unit = ToAngleUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
      if (auto unit = ToAnglePercentageUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
      if (auto unit = ToTimeUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
      if (auto unit = ToFrequencyUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
      if (auto unit = ToResolutionUnit(unitType))
      {
        CHECK(ToUnitType(*unit) == unitType);
      }
    }

    CHECK(ToLengthUnit(UnitType::QuirkyEm) == LengthUnit::QuirkyEm);
    CHECK(ToLengthUnit(UnitType::Percentage) == null);
    CHECK(ToLengthPercentageUnit(UnitType::Percentage) == LengthPercentageUnit::Percentage);
    CHECK(ToAngleUnit(UnitType::px) == null);
    CHECK(ToIntegerUnit(UnitType::Integer) == IntegerUnit::Integer);
    CHECK(ToNumberUnit(UnitType::Integer) == null);
  }

  TEST_CASE("PrimitiveUnits(length predicates)", "[CSS][Primitives][Units]")
  {
    STATIC_CHECK(IsFontRelativeLength(LengthUnit::Em));
    // Regression: QuirkyEm was missing from LengthUnit.
    STATIC_CHECK(IsFontRelativeLength(LengthUnit::QuirkyEm));
    STATIC_CHECK_FALSE(IsFontRelativeLength(LengthUnit::Rem));
    STATIC_CHECK(IsRootFontRelativeLength(LengthUnit::Rem));
    STATIC_CHECK(IsFontOrRootFontRelativeLength(LengthUnit::Rem));
    STATIC_CHECK(IsViewportPercentageLength(LengthUnit::Dvmax));
    STATIC_CHECK(IsContainerPercentageLength(LengthUnit::Cqi));
    STATIC_CHECK_FALSE(IsContainerPercentageLength(LengthUnit::Vi));

    STATIC_CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(LengthUnit::Px));
    STATIC_CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(LengthUnit::In));
    STATIC_CHECK(ConversionToCanonicalUnitRequiresConversionData(LengthUnit::Em));
    STATIC_CHECK(ConversionToCanonicalUnitRequiresConversionData(LengthUnit::QuirkyEm));
    STATIC_CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(LengthPercentageUnit::Percentage));
    STATIC_CHECK(ConversionToCanonicalUnitRequiresConversionData(LengthPercentageUnit::Vh));
    STATIC_CHECK_FALSE(ConversionToCanonicalUnitRequiresConversionData(AngleUnit::Turn));
  }

  TEST_CASE("PrimitiveUnits(casts)", "[CSS][Primitives][Units]")
  {
    STATIC_CHECK(UnitUpcast<AnglePercentageUnit>(AngleUnit::Grad) == AnglePercentageUnit::Grad);
    STATIC_CHECK(UnitUpcast<AnglePercentageUnit>(PercentageUnit::Percentage)
                 == AnglePercentageUnit::Percentage);
    STATIC_CHECK(UnitDowncast<AngleUnit>(AnglePercentageUnit::Turn) == AngleUnit::Turn);
    STATIC_CHECK(UnitCast<AngleUnit>(AngleUnit::Rad) == AngleUnit::Rad);

    STATIC_CHECK(IsUnit<LengthUnit>(LengthPercentageUnit::Em));
    STATIC_CHECK_FALSE(IsUnit<LengthUnit>(LengthPercentageUnit::Percentage));
    STATIC_CHECK(IsUnit<PercentageUnit>(LengthPercentageUnit::Percentage));

    STATIC_CHECK(DynamicUnitDowncast<LengthUnit>(LengthPercentageUnit::Vw) == LengthUnit::Vw);
    STATIC_CHECK(DynamicUnitDowncast<LengthUnit>(LengthPercentageUnit::Percentage) == null);
    STATIC_CHECK(DynamicUnitCast<LengthPercentageUnit>(LengthUnit::Cm) == LengthPercentageUnit::Cm);

    // Composite units mirror their nested units' values, so every length round trips with the same UnitType.
    for (size_t i = 0uz; i < UnitTraits<LengthUnit>::Count; ++i)
    {
      INFO(i);
      auto unit = static_cast<LengthUnit>(i);
      auto upcast = UnitUpcast<LengthPercentageUnit>(unit);
      CHECK(UnitDowncast<LengthUnit>(upcast) == unit);
      CHECK(ToUnitType(upcast) == ToUnitType(unit));
    }

    auto describe = [](LengthPercentageUnit unit)
    {
      return SwitchOnUnitType(unit, [](LengthUnit) { return 1; }, [](PercentageUnit) { return 2; });
    };
    CHECK(describe(LengthPercentageUnit::Px) == 1);
    CHECK(describe(LengthPercentageUnit::Percentage) == 2);
  }

  TEST_CASE("PrimitiveUnits(conversions)", "[CSS][Primitives][Units]")
  {
    CHECK(ConvertAngle<AngleUnit::Deg>(1.0, AngleUnit::Turn) == Approx(360.0));
    CHECK(ConvertAngle<AngleUnit::Turn>(180.0, AngleUnit::Deg) == Approx(0.5));
    CHECK(ConvertAngle<AngleUnit::Rad>(200.0, AngleUnit::Grad) == Approx(3.14159265358979));
    CHECK(ConvertAngle<AngleUnit::Grad>(90.0, AngleUnit::Deg) == Approx(100.0));
    CHECK(UnitTraits<AngleUnit>::Convert<AngleUnit::Deg>(1.0, AngleUnit::Deg) == 1.0);

    CHECK(ConvertTime<TimeUnit::Ms>(2.0, TimeUnit::S) == Approx(2000.0));
    CHECK(ConvertTime<TimeUnit::S>(250.0, TimeUnit::Ms) == Approx(0.25));

    CHECK(ConvertFrequency<FrequencyUnit::Hz>(1.5, FrequencyUnit::Khz) == Approx(1500.0));
    CHECK(ConvertFrequency<FrequencyUnit::Khz>(500.0, FrequencyUnit::Hz) == Approx(0.5));

    CHECK(ConvertResolution<ResolutionUnit::Dpi>(1.0, ResolutionUnit::Dppx) == Approx(96.0));
    CHECK(ConvertResolution<ResolutionUnit::Dppx>(2.0, ResolutionUnit::X) == Approx(2.0));
    CHECK(ConvertResolution<ResolutionUnit::Dpcm>(96.0, ResolutionUnit::Dpi) == Approx(96.0 / 2.54));
  }

  TEST_CASE("PrimitiveUnits(unit strings)", "[CSS][Primitives][Units]")
  {
    CHECK(UnitString(IntegerUnit::Integer) == u8"");
    CHECK(UnitString(PercentageUnit::Percentage) == u8"%");
    CHECK(UnitString(LengthUnit::Px) == u8"px");
    CHECK(UnitString(LengthUnit::Q) == u8"Q");
    CHECK(UnitString(LengthUnit::QuirkyEm) == u8"em");
    CHECK(UnitString(AngleUnit::Turn) == u8"turn");
    CHECK(UnitString(TimeUnit::Ms) == u8"ms");
    CHECK(UnitString(FrequencyUnit::Khz) == u8"khz");
    CHECK(UnitString(ResolutionUnit::X) == u8"x");
    CHECK(UnitString(FlexUnit::Fr) == u8"fr");
  }

  TEST_CASE("PrimitiveUnits(literals)", "[CSS][Primitives][Units]")
  {
    using namespace css::Literals;

    constexpr auto length = 10_css_px;
    STATIC_CHECK(length.Value == 10.0);
    STATIC_CHECK(decltype(length)::Unit == LengthUnit::Px);
    STATIC_CHECK(decltype(1.5_css_quirky_em)::Unit == LengthUnit::QuirkyEm);

    STATIC_CHECK((10_css_px + 5).Value == 15.0);
    STATIC_CHECK((10_css_px - 2_css_px).Value == 8.0);
    STATIC_CHECK((2 * 3_css_deg).Value == 6.0);
    STATIC_CHECK((9_css_deg / 3).Value == 3.0);
    STATIC_CHECK(1_css_s < 2_css_s);
    STATIC_CHECK(-(1_css_s) == ValueLiteral<TimeUnit::S> {-1.0});

    auto accumulated = 1_css_number;
    accumulated += 2;
    accumulated *= 3;
    accumulated -= 1_css_number;
    accumulated /= 2;
    CHECK(accumulated.Value == 4.0);
  }
}
