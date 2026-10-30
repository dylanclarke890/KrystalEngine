#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("Ratio(defaults)", "[CSS][Primitives][Ratio]")
  {
    Ratio ratio {2_css_number};

    CHECK(get<0>(ratio) == 2_css_number);
    CHECK(get<1>(ratio) == 1_css_number);
  }

  TEST_CASE("Ratio(equality)", "[CSS][Primitives][Ratio]")
  {
    CHECK(Ratio {16_css_number, 9_css_number} == Ratio {16_css_number, 9_css_number});
    CHECK(Ratio {2_css_number} == Ratio {2_css_number, 1_css_number});
    CHECK_FALSE(Ratio {16_css_number, 9_css_number} == Ratio {9_css_number, 16_css_number});
  }

  TEST_CASE("Ratio(Serialise)", "[CSS][Primitives][Ratio]")
  {
    const auto &context = DefaultSerialisationContext();

    CHECK(Serialise(context, Ratio {16_css_number, 9_css_number}) == u8"16 / 9");
    CHECK(Serialise(context, Ratio {1.5_css_number, 0.5_css_number}) == u8"1.5 / 0.5");
    // The denominator is always serialised, even when it was omitted.
    CHECK(Serialise(context, Ratio {2_css_number}) == u8"2 / 1");
  }

  TEST_CASE("Ratio(VisitValueChildren)", "[CSS][Primitives][Ratio]")
  {
    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    CHECK(VisitValueChildren(visitor, Ratio {16_css_number, 9_css_number}) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
