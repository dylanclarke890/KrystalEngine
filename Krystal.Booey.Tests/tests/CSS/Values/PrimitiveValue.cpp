#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include <numbers>

namespace krys::boo::css::tests
{
  TEST_CASE("PrimitiveValue::IsLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit :
         {UnitType::px, UnitType::cm, UnitType::mm, UnitType::in, UnitType::pt, UnitType::pc, UnitType::Q,
          UnitType::em, UnitType::ex, UnitType::rem, UnitType::lh, UnitType::cap, UnitType::ch, UnitType::ic,
          UnitType::QuirkyEm, UnitType::vw, UnitType::dvi, UnitType::cqw, UnitType::cqmax})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsLength(unit));
    }

    for (auto unit : {UnitType::Number, UnitType::Integer, UnitType::Percentage, UnitType::deg, UnitType::s,
                      UnitType::hz, UnitType::dppx, UnitType::fr, UnitType::Calc, UnitType::String,
                      UnitType::ValueId, UnitType::Unknown})
    {
      INFO(static_cast<int>(unit));
      CHECK_FALSE(PrimitiveValue::IsLength(unit));
    }
  }

  TEST_CASE("PrimitiveValue::IsFontRelativeLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit : {UnitType::em, UnitType::ex, UnitType::lh, UnitType::cap, UnitType::ch, UnitType::ic,
                      UnitType::QuirkyEm, UnitType::rem, UnitType::rex, UnitType::rlh, UnitType::rcap,
                      UnitType::rch, UnitType::ric})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsFontRelativeLength(unit));
    }

    CHECK_FALSE(PrimitiveValue::IsFontRelativeLength(UnitType::px));
    CHECK_FALSE(PrimitiveValue::IsFontRelativeLength(UnitType::vw));
    CHECK_FALSE(PrimitiveValue::IsFontRelativeLength(UnitType::Percentage));
  }

  TEST_CASE("PrimitiveValue::IsRootFontRelativeLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit :
         {UnitType::rem, UnitType::rex, UnitType::rlh, UnitType::rcap, UnitType::rch, UnitType::ric})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsRootFontRelativeLength(unit));
    }

    CHECK_FALSE(PrimitiveValue::IsRootFontRelativeLength(UnitType::em));
    CHECK_FALSE(PrimitiveValue::IsRootFontRelativeLength(UnitType::lh));
  }

  TEST_CASE("PrimitiveValue::IsFontIndependentLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit : {UnitType::px, UnitType::cm, UnitType::mm, UnitType::in, UnitType::pt, UnitType::pc})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsFontIndependentLength(unit));
    }

    CHECK_FALSE(PrimitiveValue::IsFontIndependentLength(UnitType::Q));
    CHECK_FALSE(PrimitiveValue::IsFontIndependentLength(UnitType::em));
  }

  TEST_CASE("PrimitiveValue::IsViewportPercentageLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit :
         {UnitType::vw, UnitType::vh, UnitType::vmin, UnitType::vmax, UnitType::vb, UnitType::vi,
          UnitType::svw, UnitType::svi, UnitType::lvw, UnitType::lvi, UnitType::dvw, UnitType::dvi})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsViewportPercentageLength(unit));
    }

    CHECK_FALSE(PrimitiveValue::IsViewportPercentageLength(UnitType::cqw));
    CHECK_FALSE(PrimitiveValue::IsViewportPercentageLength(UnitType::Percentage));
  }

  TEST_CASE("PrimitiveValue::IsContainerPercentageLength", "[CSS][PrimitiveValue]")
  {
    for (auto unit :
         {UnitType::cqw, UnitType::cqh, UnitType::cqi, UnitType::cqb, UnitType::cqmin, UnitType::cqmax})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::IsContainerPercentageLength(unit));
    }

    CHECK_FALSE(PrimitiveValue::IsContainerPercentageLength(UnitType::vw));
  }

  TEST_CASE("PrimitiveValue::UnitTypeString", "[CSS][PrimitiveValue]")
  {
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::px)) == "px");
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::Percentage)) == "%");
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::Q)) == "q");
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::dvmax)) == "dvmax");
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::khz)) == "khz");
    CHECK(ToNarrow(PrimitiveValue::UnitTypeString(UnitType::x)) == "x");

    // Units that don't serialise with a suffix.
    for (auto unit : {UnitType::Number, UnitType::Integer, UnitType::Dimension, UnitType::QuirkyEm,
                      UnitType::Calc, UnitType::String, UnitType::ValueId, UnitType::Unknown})
    {
      INFO(static_cast<int>(unit));
      CHECK(PrimitiveValue::UnitTypeString(unit).empty());
    }
  }

  TEST_CASE("PrimitiveValue::UnitTypeString(every dimension)", "[CSS][PrimitiveValue]")
  {
    // Every length, angle, time, frequency, resolution and flex unit has a suffix.
    for (size_t i = 0uz; i < EnumTraits<UnitType>::DistinctValues; ++i)
    {
      auto unit = static_cast<UnitType>(i);
      auto category = UnitCategoryForType(unit);
      bool hasSuffix = category != UnitCategory::Number && category != UnitCategory::Other;
      bool isContainer = PrimitiveValue::IsContainerPercentageLength(unit);
      if (hasSuffix || isContainer)
      {
        INFO(i);
        CHECK_FALSE(PrimitiveValue::UnitTypeString(unit).empty());
      }
    }
  }

  TEST_CASE("PrimitiveValue::ComputeAngle", "[CSS][PrimitiveValue]")
  {
    constexpr double pi = std::numbers::pi;

    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Deg>(UnitType::deg, 90.0) == 90.0);
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Deg>(UnitType::rad, pi) == Catch::Approx(180.0));
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Deg>(UnitType::grad, 200.0) == Catch::Approx(180.0));
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Deg>(UnitType::turn, 0.5) == Catch::Approx(180.0));

    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Rad>(UnitType::deg, 180.0) == Catch::Approx(pi));
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Rad>(UnitType::turn, 1.0) == Catch::Approx(2.0 * pi));

    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Grad>(UnitType::deg, 90.0) == Catch::Approx(100.0));
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Grad>(UnitType::rad, pi) == Catch::Approx(200.0));

    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Turn>(UnitType::deg, 720.0) == Catch::Approx(2.0));
    CHECK(PrimitiveValue::ComputeAngle<AngleUnit::Turn>(UnitType::grad, 100.0) == Catch::Approx(0.25));
  }

  TEST_CASE("PrimitiveValue::ComputeTime", "[CSS][PrimitiveValue]")
  {
    CHECK(PrimitiveValue::ComputeTime<TimeUnit::S>(UnitType::s, 2.0) == 2.0);
    CHECK(PrimitiveValue::ComputeTime<TimeUnit::S>(UnitType::ms, 250.0) == Catch::Approx(0.25));
    CHECK(PrimitiveValue::ComputeTime<TimeUnit::Ms>(UnitType::s, 1.5) == Catch::Approx(1500.0));
    CHECK(PrimitiveValue::ComputeTime<TimeUnit::Ms>(UnitType::ms, 30.0) == 30.0);
  }

  TEST_CASE("PrimitiveValue::ComputeFrequency", "[CSS][PrimitiveValue]")
  {
    CHECK(PrimitiveValue::ComputeFrequency<FrequencyUnit::Hz>(UnitType::hz, 440.0) == 440.0);
    CHECK(PrimitiveValue::ComputeFrequency<FrequencyUnit::Hz>(UnitType::khz, 2.0) == 2000.0);
    CHECK(PrimitiveValue::ComputeFrequency<FrequencyUnit::Khz>(UnitType::hz, 500.0) == Catch::Approx(0.5));
    CHECK(PrimitiveValue::ComputeFrequency<FrequencyUnit::Khz>(UnitType::khz, 3.0) == 3.0);
  }

  TEST_CASE("PrimitiveValue::ComputeResolution", "[CSS][PrimitiveValue]")
  {
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dppx>(UnitType::dppx, 2.0) == 2.0);
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dppx>(UnitType::x, 2.0) == 2.0);
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dppx>(UnitType::dpi, 192.0)
          == Catch::Approx(2.0));
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dppx>(UnitType::dpcm, 96.0 / 2.54)
          == Catch::Approx(1.0));

    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::X>(UnitType::dpi, 96.0) == Catch::Approx(1.0));

    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dpi>(UnitType::dppx, 1.0) == Catch::Approx(96.0));
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dpi>(UnitType::dpcm, 1.0) == Catch::Approx(2.54));

    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dpcm>(UnitType::dpi, 2.54) == Catch::Approx(1.0));
    CHECK(PrimitiveValue::ComputeResolution<ResolutionUnit::Dpcm>(UnitType::x, 1.0)
          == Catch::Approx(96.0 / 2.54));
  }

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
}
