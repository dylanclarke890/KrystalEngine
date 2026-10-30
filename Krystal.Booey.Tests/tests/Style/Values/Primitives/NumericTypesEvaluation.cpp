#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericOrKeywordConversions.hpp"

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  namespace
  {
    using PercentageFloat = Percentage<css::All, float>;

    LengthPercentage<> CalcLengthPercentage(double length, double percentage)
    {
      return LengthPercentage<>::Calc {
        calc::Add(calc::CreateDimension(length), calc::CreatePercentage(percentage))};
    }
  }

  TEST_CASE("Style evaluation: leaf types", "[Style][NumericTypesEvaluation]")
  {
    CHECK(Evaluate<float>(Length<> {12.0f}, ZoomNeeded {}) == 12.0f);
    CHECK(Evaluate<float>(Length<> {12.0f}, 100.0f, ZoomNeeded {}) == 12.0f);
    CHECK(Evaluate<float>(Length<css::AllUnzoomed> {12.0f}, ZoomFactor {2.0f, 1.0f}) == 24.0f);
    CHECK(Evaluate<float>(Percentage<> {50}) == 0.5f);
    CHECK(Evaluate<float>(Percentage<> {50}, 200.0f) == 100.0f);
    CHECK(Evaluate<double>(Number<> {2.5}) == 2.5);
    CHECK(Evaluate<double>(Angle<> {90}, 1.0) == 90.0);
    CHECK(Evaluate<double>(NumberOrPercentageResolvedToNumber<> {Percentage<> {50}}) == 0.5);
    CHECK(Evaluate<LayoutUnit>(Percentage<> {50}, LayoutUnit(10)) == LayoutUnit(5));
  }

  TEST_CASE("Style evaluation: length-percentage", "[Style][NumericTypesEvaluation]")
  {
    CHECK(Evaluate<float>(LengthPercentage<> {Length<> {4.0f}}, 100.0f, ZoomNeeded {}) == 4.0f);
    CHECK(Evaluate<float>(LengthPercentage<> {PercentageFloat {25.0f}}, 200.0f, ZoomNeeded {}) == 50.0f);
    CHECK(Evaluate<float>(CalcLengthPercentage(10, 50), 100.0f, ZoomNeeded {}) == 60.0f);
    CHECK(Evaluate<float>(LengthPercentage<css::AllUnzoomed> {Length<css::AllUnzoomed> {4.0f}}, 100.0f,
                          ZoomFactor {2.0f, 1.0f})
          == 8.0f);
    CHECK(Evaluate<LayoutUnit>(LengthPercentage<> {PercentageFloat {50.0f}}, LayoutUnit(20), ZoomNeeded {})
          == LayoutUnit(10));
  }

  TEST_CASE("Style evaluation: points and sizes", "[Style][NumericTypesEvaluation]")
  {
    const LengthPercentageSpaceSeparatedPointAll point {LengthPercentage<> {Length<> {5.0f}},
                                                        LengthPercentage<> {PercentageFloat {50.0f}}};
    CHECK(Evaluate<FloatPoint>(point, FloatSize {100.0f, 40.0f}, ZoomNeeded {}) == FloatPoint {5.0f, 20.0f});
    CHECK(Evaluate<LayoutPoint>(point, LayoutSize {100, 40}, ZoomNeeded {}) == LayoutPoint {5, 20});

    const LengthPercentageSpaceSeparatedSizeAll size {LengthPercentage<> {PercentageFloat {25.0f}},
                                                      LengthPercentage<> {Length<> {3.0f}}};
    CHECK(Evaluate<FloatSize>(size, FloatSize {100.0f, 40.0f}, ZoomNeeded {}) == FloatSize {25.0f, 3.0f});
    CHECK(Evaluate<LayoutSize>(size, LayoutSize {100, 40}, ZoomNeeded {}) == LayoutSize {25, 3});

    const LengthPercentageMinimallySerialisingSpaceSeparatedSizeAll minimal {
      LengthPercentage<> {PercentageFloat {25.0f}}, LengthPercentage<> {PercentageFloat {25.0f}}};
    CHECK(Evaluate<FloatSize>(minimal, FloatSize {100.0f, 40.0f}, ZoomNeeded {}) == FloatSize {25.0f, 10.0f});
  }

  TEST_CASE("Style evaluation: reflection", "[Style][NumericTypesEvaluation]")
  {
    CHECK(Reflect(LengthPercentage<> {Length<> {0.0f}}) == LengthPercentage<> {100_css_percentage});
    CHECK(Reflect(LengthPercentage<> {PercentageFloat {30.0f}}) == LengthPercentage<> {70_css_percentage});

    auto reflectedLength = Reflect(LengthPercentage<> {Length<> {10.0f}});
    REQUIRE(reflectedLength.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK(Evaluate<float>(reflectedLength, 200.0f, ZoomNeeded {}) == 190.0f);

    auto reflectedCalc = Reflect(CalcLengthPercentage(10, 10));
    CHECK(Evaluate<float>(reflectedCalc, 100.0f, ZoomNeeded {}) == 80.0f);

    // Sums.
    const LengthPercentage<> zero {Length<> {0.0f}};
    const LengthPercentage<css::NonNegative> nonNegativeZero {Length<css::NonNegative> {0.0f}};
    auto bothZero = ReflectSum(zero, nonNegativeZero);
    STATIC_CHECK((SameType<decltype(bothZero), LengthPercentage<css::All>>));
    CHECK(bothZero == LengthPercentage<> {100_css_percentage});

    CHECK(ReflectSum(zero, LengthPercentage<> {PercentageFloat {20.0f}})
          == LengthPercentage<> {80_css_percentage});
    CHECK(ReflectSum(LengthPercentage<> {PercentageFloat {20.0f}}, zero)
          == LengthPercentage<> {80_css_percentage});
    CHECK(
      ReflectSum(LengthPercentage<> {PercentageFloat {20.0f}}, LengthPercentage<> {PercentageFloat {30.0f}})
      == LengthPercentage<> {50_css_percentage});

    auto mixed =
      ReflectSum(LengthPercentage<> {Length<> {10.0f}}, LengthPercentage<> {PercentageFloat {30.0f}});
    REQUIRE(mixed.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK(Evaluate<float>(mixed, 100.0f, ZoomNeeded {}) == 60.0f);

    auto onlyB = ReflectSum(zero, LengthPercentage<> {Length<> {10.0f}});
    CHECK(Evaluate<float>(onlyB, 100.0f, ZoomNeeded {}) == 90.0f);
  }

  TEST_CASE("Style blending: numerics", "[Style][NumericTypesBlending]")
  {
    const BlendingContext quarter(0.25);
    CHECK(Blend(Number<> {0}, Number<> {4}, quarter) == Number<> {1});
    CHECK(Blend(Number<css::NonNegative> {0}, Number<css::NonNegative> {4}, BlendingContext(-1.0))
          == Number<css::NonNegative> {0});
    CHECK(Blend(Length<> {0.0f}, Length<> {8.0f}, quarter) == Length<> {2.0f});
    CHECK(Blend(Integer<> {0}, Integer<> {4}, BlendingContext(0.5)) == Integer<> {2});
    CHECK(Blend(Number<> {1}, Number<> {4}, BlendingContext(0)) == Number<> {1});
    CHECK(Blend(Number<> {1}, Number<> {4}, BlendingContext(1)) == Number<> {4});

    const NumberOrPercentageResolvedToNumber<> from {Number<> {0}};
    const NumberOrPercentageResolvedToNumber<> to {Percentage<> {100}};
    CHECK(Blend(from, to, BlendingContext(0.5)) == 0.5);
    CHECK(CanBlend(from, to));
  }

  TEST_CASE("Style blending: length-percentage", "[Style][NumericTypesBlending]")
  {
    const BlendingContext half(0.5);
    const LengthPercentage<> length0 {Length<> {0.0f}};
    const LengthPercentage<> length10 {Length<> {10.0f}};
    const LengthPercentage<> percentage0 {PercentageFloat {0.0f}};
    const LengthPercentage<> percentage40 {PercentageFloat {40.0f}};

    // Same alternative.
    CHECK(Blend(length0, length10, half) == LengthPercentage<> {Length<> {5.0f}});
    CHECK(Blend(percentage0, percentage40, half) == LengthPercentage<> {PercentageFloat {20.0f}});
    CHECK_FALSE(RequiresInterpolationForAccumulativeIteration(length0, length10));
    CHECK(RequiresInterpolationForAccumulativeIteration(length0, percentage40));

    // Zero to zero of a different kind.
    CHECK(Blend(percentage0, length0, half) == LengthPercentage<> {0_css_px});

    // From a zero to the other kind blends just the other kind.
    CHECK(Blend(length0, percentage40, half) == LengthPercentage<> {PercentageFloat {20.0f}});
    CHECK(Blend(percentage40, length0, half) == LengthPercentage<> {PercentageFloat {20.0f}});

    // Mixed non-zero values become calc.
    auto mixed = Blend(length10, percentage40, half);
    REQUIRE(mixed.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK(Evaluate<float>(mixed, 100.0f, ZoomNeeded {}) == 25.0f);

    // Accumulation adds instead.
    auto added = Blend(length10, percentage40, BlendingContext(0.5, false, CompositeOperation::Add));
    REQUIRE(added.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK(Evaluate<float>(added, 100.0f, ZoomNeeded {}) == 50.0f);
  }

  TEST_CASE("Style PrimitiveNumericOrKeyword conversions", "[Style][PrimitiveNumericOrKeyword]")
  {
    using CSSOrAuto = css::PrimitiveNumericOrKeyword<css::LengthPercentage<>, css::keywords::Auto>;
    using StyleOrAuto = PrimitiveNumericOrKeyword<LengthPercentage<>, css::keywords::Auto>;
    STATIC_CHECK((SameType<StyleType<CSSOrAuto>, StyleOrAuto>));
    STATIC_CHECK((SameType<CSSType<StyleOrAuto>, CSSOrAuto>));

    auto length = ConvertToStyle(CSSOrAuto {4_css_px}, css::NoConversionDataRequiredTag {});
    CHECK(length.HoldsAlternative<Length<>>());
    auto keyword = ConvertToStyle(CSSOrAuto {css::keywords::Auto {}}, css::ToLengthConversionData {});
    CHECK(keyword.HoldsAlternative<css::keywords::Auto>());
  }
}
