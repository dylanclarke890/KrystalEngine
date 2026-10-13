#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  namespace
  {
    template <typename T>
    T CalcOf(double dimension, double percentage)
    {
      return T {calc::Add(calc::CreateDimension(dimension), calc::CreatePercentage(percentage))};
    }
  }

  TEST_CASE("Style PrimitiveNumeric: non-composite values", "[Style][PrimitiveNumeric]")
  {
    constexpr Number<> number {2.5};
    STATIC_CHECK(number.Value == 2.5);
    STATIC_CHECK(number == 2.5);
    STATIC_CHECK(Number<> {3_css_number}.Value == 3.0);
    STATIC_CHECK(Number<> {3_css_integer}.Value == 3.0);
    STATIC_CHECK(Integer<> {3.7_css_number}.Value == 3);
    STATIC_CHECK(Number<>::Unit == css::NumberUnit::Number);

    STATIC_CHECK(Number<> {0}.IsZero());
    STATIC_CHECK(Number<> {0}.IsKnownZero());
    STATIC_CHECK(Number<> {1}.IsPositive());
    STATIC_CHECK(Number<> {-1}.IsNegative());
    STATIC_CHECK(Number<> {0}.IsPositiveOrZero());
    STATIC_CHECK(Number<> {0}.IsNegativeOrZero());
    STATIC_CHECK(Number<> {1} < Number<> {2});
    STATIC_CHECK(css::HasIsZero<Number<>>);
    STATIC_CHECK_FALSE(css::HasIsZero<Number<css::Range {1, 2}>>);

    STATIC_CHECK(Numeric<Number<>>);
    STATIC_CHECK(NonCompositeNumeric<Angle<>>);
    STATIC_CHECK(DimensionPercentageNumeric<LengthPercentage<>>);
    STATIC_CHECK(IsPercentage<Percentage<>>);
    STATIC_CHECK_FALSE(IsPercentage<Number<>>);

  }

  TEST_CASE("Style PrimitiveNumeric: lengths and zoom", "[Style][PrimitiveNumeric]")
  {
    constexpr Length<> zoomed {10.0f};
    STATIC_CHECK(zoomed.ResolveZoom(ZoomNeeded {}) == 10.0f);
    STATIC_CHECK(zoomed.UnresolvedValue() == 10.0f);
    STATIC_CHECK(Length<> {5_css_px}.UnresolvedValue() == 5.0f);

    constexpr Length<css::AllUnzoomed> unzoomed {10.0f};
    STATIC_CHECK(unzoomed.ResolveZoom(ZoomFactor {2.0f, 1.0f}) == 20.0f);
    STATIC_CHECK(unzoomed.UnresolvedValue() == 10.0f);
    STATIC_CHECK(Length<> {0.0f}.IsZero());
  }

  TEST_CASE("Style PrimitiveNumeric: markable", "[Style][PrimitiveNumeric]")
  {
    Markable<Number<>> empty;
    CHECK_FALSE(empty);
    empty = Number<> {1.0};
    CHECK(empty);
    CHECK(empty->Value == 1.0);

    // Integers with a non-negative minimum use -1 as the empty value.
    Markable<Integer<css::NonNegative>> integer;
    CHECK_FALSE(integer);
    integer = Integer<css::NonNegative> {0};
    CHECK(integer);

    Markable<Length<>> length;
    CHECK_FALSE(length);
    length = Length<> {0.0f};
    CHECK(length);
  }

  TEST_CASE("Style PrimitiveNumeric: dimension-percentage", "[Style][PrimitiveNumeric]")
  {
    STATIC_CHECK(sizeof(LengthPercentage<>) == sizeof(uint64));

    LengthPercentage<> length {Length<> {4.0f}};
    CHECK(length.HoldsAlternative<Length<>>());
    CHECK(get<Length<>>(length) == Length<> {4.0f});
    CHECK(length.IsKnownZero() == false);
    CHECK(LengthPercentage<> {Length<> {0.0f}}.IsKnownZero());

    LengthPercentage<> percentage {50_css_percentage};
    CHECK(percentage.HoldsAlternative<Percentage<css::All, float>>());
    CHECK(percentage.SwitchOn([](const Length<> &) { return 0; },
                              [](const Percentage<css::All, float> &) { return 1; },
                              [](const UnevaluatedCalc<css::LengthPercentage<>> &) { return 2; })
          == 1);
    CHECK(LengthPercentage<> {10_css_px}
          == length.SwitchOn([](const auto &) { return LengthPercentage<> {10_css_px}; }));

    auto calc = CalcOf<LengthPercentage<>::Calc>(1.0, 50.0);
    LengthPercentage<> withCalc {calc};
    CHECK(withCalc.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK_FALSE(withCalc.IsKnownZero());
    CHECK(withCalc == LengthPercentage<> {CalcOf<LengthPercentage<>::Calc>(1.0, 50.0)});
    CHECK_FALSE(withCalc == length);

    // Copies share the calculation; moves leave the source valueless.
    LengthPercentage<> copy = withCalc;
    CHECK(copy == withCalc);
    LengthPercentage<> moved = krys::move(copy);
    CHECK(moved == withCalc);
    copy = length;
    CHECK(copy == length);
  }

  TEST_CASE("Style NumberOrPercentage", "[Style][PrimitiveNumeric]")
  {
    NumberOrPercentage<> number {Number<> {2.0}};
    CHECK(number.SwitchOn([](const Number<> &) { return true; }, [](const Percentage<> &) { return false; }));
    NumberOrPercentage<> percentage {Variant<Number<>, Percentage<>> {Percentage<> {50.0}}};
    CHECK(
      percentage.SwitchOn([](const Number<> &) { return false; }, [](const Percentage<> &) { return true; }));
    CHECK_FALSE(number == percentage);

    Markable<NumberOrPercentage<>> markable;
    CHECK_FALSE(markable);
    markable = number;
    CHECK(markable);

    constexpr NumberOrPercentageResolvedToNumber<> resolved {Percentage<> {50.0}};
    STATIC_CHECK(resolved == 0.5);
    STATIC_CHECK(NumberOrPercentageResolvedToNumber<> {25_css_percentage} == 0.25);
    STATIC_CHECK(NumberOrPercentageResolvedToNumber<> {2_css_number} == 2.0);
    STATIC_CHECK(NumberOrPercentageResolvedToNumber<> {0.0}.IsZero());
  }

  TEST_CASE("Style PrimitiveNumericOrKeyword", "[Style][PrimitiveNumeric]")
  {
    using LengthPercentageOrAuto = PrimitiveNumericOrKeyword<LengthPercentage<>, css::keywords::Auto>;
    STATIC_CHECK((SameType<LengthPercentageOrAuto::Representation::Representation,
                          CompactVariant<Length<>, Percentage<css::All, float>,
                                         UnevaluatedCalc<css::LengthPercentage<>>, css::keywords::Auto>>));

    LengthPercentageOrAuto length {LengthPercentage<> {Length<> {0.0f}}};
    CHECK(length.HoldsAlternative<Length<>>());
    CHECK(length.IsZero());

    LengthPercentageOrAuto keyword {css::keywords::Auto {}};
    CHECK(keyword.HoldsAlternative<css::keywords::Auto>());
    CHECK_FALSE(keyword.IsZero());
    CHECK_FALSE(keyword == length);
  }
}
