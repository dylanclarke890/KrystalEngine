#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/ValueMap.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style::tests
{
  struct TestSize : LengthWrapperBase<LengthPercentage<>, css::keywords::Auto, css::keywords::MinContent>
  {
    using Base::Base;
  };
}

namespace krys::boo::css
{
  template <>
  constexpr bool TreatAsVariantLike<style::tests::TestSize> = true;
}

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  namespace
  {
    using PercentageFloat = Percentage<css::All, float>;

    TestSize::Calc CalcOf(double length, double percentage)
    {
      return TestSize::Calc {calc::Add(calc::CreateDimension(length), calc::CreatePercentage(percentage))};
    }
  }

  TEST_CASE("Style calc ValueMap", "[Style][Calc][ValueMap]")
  {
    auto &map = calc::ValueMap::CalculationValues();
    auto value =
      calc::Value::Create(css::Category::Length, css::All, calc::Tree {.Root = calc::CreateDimension(1)});

    auto first = map.Insert(ShareRef(*value));
    auto second = map.Insert(ShareRef(*value));
    CHECK(first != 0);
    CHECK(second != first);
    CHECK(&map.Get(first) == value.get());

    map.AddRef(first);
    map.SubRef(first);
    CHECK(&map.Get(first) == value.get());
    map.SubRef(first);
    map.SubRef(second);
  }

  TEST_CASE("Style LengthWrapperData", "[Style][LengthWrapper]")
  {
    LengthWrapperData fixed {1, 4.0f};
    CHECK(fixed.Type() == 1);
    CHECK(fixed.Value() == 4.0f);
    CHECK_FALSE(fixed.HasQuirk());
    CHECK(fixed == LengthWrapperData {1, 4.0f});
    CHECK_FALSE(fixed == LengthWrapperData {1, 4.0f, true});
    CHECK_FALSE(fixed == LengthWrapperData {2, 4.0f});
    CHECK(fixed.IsKnownPositive(LengthWrapperDataEvaluationKind::Fixed));
    CHECK_FALSE(fixed.IsKnownPositive(LengthWrapperDataEvaluationKind::Calculation));
    CHECK(fixed.IsPossiblyNegative(LengthWrapperDataEvaluationKind::Flag));
    CHECK(LengthWrapperData {1, 0.0f}.IsKnownZero(LengthWrapperDataEvaluationKind::Percentage));

    auto value = calc::Value::Create(
      css::Category::LengthPercentage, css::All,
      calc::Tree {.Root = calc::Add(calc::CreateDimension(10), calc::CreatePercentage(50))});
    LengthWrapperData calculated {3, ShareRef(*value)};
    CHECK(&calculated.CalculationValue() == value.get());

    {
      // Copies share the handle; the value stays alive until the last copy goes.
      LengthWrapperData copy = calculated;
      CHECK(copy == calculated);
      LengthWrapperData moved = krys::move(copy);
      CHECK(moved == calculated);
      copy = moved;
      CHECK(copy == calculated);
      copy = fixed;
      CHECK(copy == fixed);
    }

    CHECK(&calculated.CalculationValue() == value.get());
    CHECK(calculated.ValueForLengthWrapperDataWithLazyMaximum<float, float>(
            LengthWrapperDataEvaluationKind::Calculation, [] { return 100.0f; }, ZoomNeeded {})
          == 60.0f);
    CHECK(fixed.ValueForLengthWrapperDataWithLazyMaximum<float, float>(
            LengthWrapperDataEvaluationKind::Fixed, [] { return 100.0f; }, ZoomFactor {2.0f, 1.0f})
          == 8.0f);
    CHECK(fixed.ValueForLengthWrapperDataWithLazyMaximum<float, float>(
            LengthWrapperDataEvaluationKind::Percentage, [] { return 50.0f; }, ZoomNeeded {})
          == 2.0f);
    CHECK(fixed.ValueForLengthWrapperDataWithLazyMaximum<float, float>(
            LengthWrapperDataEvaluationKind::Flag, [] { return 50.0f; }, ZoomNeeded {})
          == 50.0f);
    CHECK(fixed.MinimumValueForLengthWrapperDataWithLazyMaximum<float, float>(
            LengthWrapperDataEvaluationKind::Flag, [] { return 50.0f; }, ZoomNeeded {})
          == 0.0f);

    LengthWrapperData empty {HashTableEmptyValueTag {}};
    LengthWrapperData deleted {HashTableDeletedValueTag {}};
    CHECK(empty.Type() == 0);
  }

  TEST_CASE("Style LengthWrapperBase", "[Style][LengthWrapper]")
  {
    STATIC_CHECK(TestSize::IndexForFixed == 2);
    STATIC_CHECK(TestSize::IndexForPercentage == 3);
    STATIC_CHECK(TestSize::IndexForCalc == 4);
    STATIC_CHECK(TestSize::SupportsAuto);
    STATIC_CHECK(TestSize::SupportsMinContent);
    STATIC_CHECK_FALSE(TestSize::SupportsNone);
    STATIC_CHECK(LengthWrapperBaseDerived<TestSize>);
    STATIC_CHECK_FALSE(LengthWrapperBaseDerived<LengthPercentage<>>);

    TestSize autoSize {css::keywords::Auto {}};
    CHECK(autoSize.HoldsAlternative<css::keywords::Auto>());
    CHECK_FALSE(autoSize.IsSpecified());
    CHECK(autoSize.IsPossiblyZero());
    CHECK_FALSE(autoSize.IsKnownZero());
    CHECK(autoSize.SwitchOn([](css::keywords::Auto) { return 1; }, [](const auto &) { return 0; }) == 1);

    TestSize minContent {css::keywords::MinContent {}};
    CHECK(minContent.HoldsAlternative<css::keywords::MinContent>());
    CHECK_FALSE(minContent.HasSameType(autoSize));

    TestSize fixed {10_css_px};
    CHECK(fixed.IsFixed());
    CHECK(fixed.IsKnownPositive());
    CHECK(fixed.TryFixed() == Length<> {10.0f});
    CHECK_FALSE(fixed.TryPercentage());
    CHECK(fixed.HoldsAlternative<LengthPercentage<>>());
    CHECK(fixed.TrySpecified() == LengthPercentage<> {Length<> {10.0f}});

    TestSize quirky {Length<> {10.0f}, true};
    CHECK(quirky.HasQuirk());
    CHECK_FALSE(quirky == fixed);

    TestSize percent {50_css_percentage};
    CHECK(percent.IsPercent());
    CHECK(percent.IsPercentOrCalculated());
    CHECK(percent.TryPercentage() == PercentageFloat {50.0f});

    TestSize calculated {CalcOf(10, 50)};
    CHECK(calculated.IsCalculated());
    CHECK(calculated.TryCalc());
    CHECK_FALSE(calculated.IsKnownZero());
    CHECK(calculated.IsPossiblyZero());
    CHECK(calculated == TestSize {CalcOf(10, 50)});

    TestSize fromSpecified {LengthPercentage<> {PercentageFloat {25.0f}}};
    CHECK(fromSpecified.IsPercent());
  }

  TEST_CASE("Style LengthWrapper evaluation", "[Style][LengthWrapper]")
  {
    CHECK(Evaluate<float>(TestSize {10_css_px}, 100.0f, ZoomNeeded {}) == 10.0f);
    CHECK(Evaluate<float>(TestSize {50_css_percentage}, 100.0f, ZoomNeeded {}) == 50.0f);
    CHECK(Evaluate<float>(TestSize {CalcOf(10, 50)}, [] { return 100.0f; }, ZoomNeeded {}) == 60.0f);
    CHECK(Evaluate<float>(TestSize {css::keywords::Auto {}}, 100.0f, ZoomNeeded {}) == 100.0f);
    CHECK(Evaluate<LayoutUnit>(TestSize {50_css_percentage}, LayoutUnit(20), ZoomNeeded {})
          == LayoutUnit(10));

    CHECK(EvaluateMinimum<LayoutUnit>(TestSize {css::keywords::Auto {}}, LayoutUnit(100), ZoomNeeded {})
          == LayoutUnit(0));
    CHECK(EvaluateMinimum<LayoutUnit>(TestSize {50_css_percentage}, LayoutUnit(100), ZoomNeeded {})
          == LayoutUnit(50));
  }

  TEST_CASE("Style LengthWrapper blending", "[Style][LengthWrapper]")
  {
    const BlendingContext half(0.5);
    const TestSize autoSize {css::keywords::Auto {}};
    const TestSize fixed0 {0_css_px};
    const TestSize fixed10 {10_css_px};
    const TestSize percent40 {40_css_percentage};

    CHECK(CanBlend(fixed0, fixed10));
    CHECK(CanBlend(fixed10, percent40));
    CHECK_FALSE(CanBlend(autoSize, fixed10));
    CHECK(RequiresInterpolationForAccumulativeIteration(fixed10, percent40));
    CHECK_FALSE(RequiresInterpolationForAccumulativeIteration(fixed0, fixed10));

    // Keywords swap discretely.
    CHECK(Blend(autoSize, fixed10, BlendingContext(0.25)) == autoSize);
    CHECK(Blend(autoSize, fixed10, BlendingContext(0.75)) == fixed10);

    CHECK(Blend(fixed0, fixed10, half) == TestSize {5_css_px});
    CHECK(Blend(TestSize {0_css_percentage}, percent40, half) == TestSize {20_css_percentage});

    // From a zero length to a percentage blends just the percentage.
    CHECK(Blend(fixed0, percent40, half) == TestSize {20_css_percentage});

    auto mixed = Blend(fixed10, percent40, half);
    REQUIRE(mixed.IsCalculated());
    CHECK(Evaluate<float>(mixed, 100.0f, ZoomNeeded {}) == 25.0f);

    auto added = Blend(fixed10, percent40, BlendingContext(0.5, false, CompositeOperation::Add));
    REQUIRE(added.IsCalculated());
    CHECK(Evaluate<float>(added, 100.0f, ZoomNeeded {}) == 50.0f);
  }
}
