#include "Krystal.Booey/CSS/Values/Primitives/NumericRaw.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveKeywordList.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::css::tests
{
  TEST_CASE("Range(constants)", "[CSS][Primitives][Range]")
  {
    STATIC_CHECK(All.Min == -Range::Inf);
    STATIC_CHECK(All.Max == Range::Inf);
    STATIC_CHECK(NonNegative == Range {0, Range::Inf});
    STATIC_CHECK(Positive.Min == 1);
    STATIC_CHECK(ClosedUnitRangeClampUpper.ClampOptions == RangeClampOptions::ClampUpper);
    STATIC_CHECK(ClosedPercentageRangeUnzoomed.ZoomOptions == RangeZoomOptions::Unzoomed);
    STATIC_CHECK(ClosedUnitRange != ClosedUnitRangeClampBoth);
  }

  TEST_CASE("Range(ClampToRange)", "[CSS][Primitives][Range]")
  {
    STATIC_CHECK(ClampToRange<ClosedUnitRange, float>(1.5) == 1.0f);
    STATIC_CHECK(ClampToRange<ClosedUnitRange, float>(-0.5) == 0.0f);
    STATIC_CHECK(ClampToRange<ClosedUnitRange, double>(0.25) == 0.25);
    STATIC_CHECK(ClampToRange<All, float>(1e300) == std::numeric_limits<float>::max());
    STATIC_CHECK(ClampToRange<NonNegative, float>(5.0, 1.0f, 2.0f) == 2.0f);

    STATIC_CHECK(ClampToRange<Positive, int32>(0) == 1);
    STATIC_CHECK(ClampToRange<Positive, int32>(0.5) == 1);
    STATIC_CHECK(ClampToRange<All, int32>(5) == 5);
    STATIC_CHECK(ClampToRange<ClosedPercentageRange, int32>(150) == 100);
    STATIC_CHECK(ClampToRange<NonNegative, uint32>(-5.0) == 0u);
    STATIC_CHECK(ClampToRange<ClosedPercentageRange, uint32>(500.0) == 100u);
  }

  TEST_CASE("Range(IsWithinRange)", "[CSS][Primitives][Range]")
  {
    CHECK(IsWithinRange<NonNegative>(0.0));
    CHECK_FALSE(IsWithinRange<NonNegative>(-0.1));
    CHECK_FALSE(IsWithinRange<All>(std::numeric_limits<double>::quiet_NaN()));
    CHECK(IsWithinRange<ClosedUnitRange>(1.0f));
    CHECK_FALSE(IsWithinRange<ClosedUnitRange>(1.5f));

    STATIC_CHECK(IsWithinRange<Positive>(int32 {1}));
    STATIC_CHECK_FALSE(IsWithinRange<Positive>(int32 {0}));
    STATIC_CHECK(IsWithinRange<All>(int32 {-5}));
    STATIC_CHECK(IsWithinRange<ClosedPercentageRange>(uint32 {100}));
    STATIC_CHECK_FALSE(IsWithinRange<ClosedPercentageRange>(uint32 {101}));
  }

  TEST_CASE("NumericRaw(dimensions)", "[CSS][Primitives][NumericRaw]")
  {
    using namespace css::Literals;

    constexpr LengthRaw<> length {LengthUnit::Em, 2};
    STATIC_CHECK(length.Unit == LengthUnit::Em);
    STATIC_CHECK(length.Value == 2.0);
    STATIC_CHECK(LengthRaw<>::Category == Category::Length);
    STATIC_CHECK(LengthRaw<>::Range == All);
    STATIC_CHECK(ResolutionRaw<>::Range == NonNegative);
    STATIC_CHECK(SameType<LengthRaw<>::ResolvedValueType, float>);

    constexpr LengthRaw<> fromLiteral {10_css_px};
    STATIC_CHECK(fromLiteral == LengthRaw<> {LengthUnit::Px, 10});
    STATIC_CHECK(fromLiteral == 10_css_px);
    STATIC_CHECK_FALSE(fromLiteral == 10_css_em);

    STATIC_CHECK(NumericRaw<LengthRaw<>>);
    STATIC_CHECK(NumericRaw<AnglePercentageRaw<ClosedUnitRange>>);
    STATIC_CHECK_FALSE(NumericRaw<double>);
  }

  TEST_CASE("NumericRaw(composites)", "[CSS][Primitives][NumericRaw]")
  {
    using namespace css::Literals;

    constexpr LengthPercentageRaw<> percentage {PercentageUnit::Percentage, 50};
    STATIC_CHECK(percentage.Unit == LengthPercentageUnit::Percentage);
    STATIC_CHECK(percentage.Value == 50.0);

    constexpr LengthPercentageRaw<> fromRaw {LengthRaw<> {LengthUnit::Rem, 3}};
    STATIC_CHECK(fromRaw.Unit == LengthPercentageUnit::Rem);
    STATIC_CHECK(fromRaw == LengthRaw<> {LengthUnit::Rem, 3});

    constexpr AnglePercentageRaw<> fromNestedLiteral {90_css_deg};
    STATIC_CHECK(fromNestedLiteral.Unit == AnglePercentageUnit::Deg);
    STATIC_CHECK(fromNestedLiteral == 90_css_deg);
    STATIC_CHECK(AnglePercentageRaw<> {25_css_percentage} == 25_css_percentage);

    STATIC_CHECK(LengthPercentageRaw<>::Category == Category::LengthPercentage);
  }

  TEST_CASE("NumericRaw(single unit)", "[CSS][Primitives][NumericRaw]")
  {
    using namespace css::Literals;

    constexpr NumberRaw<> number {2.5};
    STATIC_CHECK(number.Value == 2.5);
    STATIC_CHECK(NumberRaw<>::Unit == NumberUnit::Number);
    STATIC_CHECK(number == 2.5_css_number);

    constexpr IntegerRaw<> integer {7};
    STATIC_CHECK(integer.Value == 7.0);
    STATIC_CHECK(SameType<IntegerRaw<>::ResolvedValueType, int>);

    STATIC_CHECK(PercentageRaw<> {PercentageUnit::Percentage, 10} == PercentageRaw<> {10});
    STATIC_CHECK(FlexRaw<> {1_css_fr}.Value == 1.0);
  }

  TEST_CASE("PrimitiveKeywordList", "[CSS][Primitives][KeywordList]")
  {
    using List = PrimitiveKeywordList<keywords::Auto, keywords::None>;

    STATIC_CHECK(List::Count == 2);
    STATIC_CHECK(List::IsValidKeyword(keywords::Auto {}));
    STATIC_CHECK_FALSE(List::IsValidKeyword(keywords::Calc {}));
    STATIC_CHECK(List::OffsetForKeyword(keywords::None {}) == 1);
    STATIC_CHECK(ValidKeywordForList<keywords::Auto, List>);
    STATIC_CHECK_FALSE(ValidKeywordForList<keywords::Calc, List>);
    STATIC_CHECK(PrimitiveKeyword<keywords::Auto>);
    STATIC_CHECK_FALSE(PrimitiveKeyword<int>);

    CHECK(List::VisitKeywordAtOffset(1, [](auto keyword) { return decltype(keyword)::Value; })
          == ValueId::None);

    STATIC_CHECK(PrimitiveKeywordList<>::Count == 0);
    STATIC_CHECK_FALSE(PrimitiveKeywordList<>::IsValidKeyword(keywords::Auto {}));
  }
}
