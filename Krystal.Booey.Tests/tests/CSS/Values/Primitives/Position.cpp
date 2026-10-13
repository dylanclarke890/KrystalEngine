#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Position TwoComponent(TwoComponentPositionHorizontal x, TwoComponentPositionVertical y)
    {
      return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
    }

    CSSOMString SerialiseForTest(const auto &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }
  }

  TEST_CASE("Position(IsCenterPosition)", "[CSS][Primitives][Position]")
  {
    CHECK(IsCenterPosition(TwoComponent({keywords::Center {}}, {keywords::Center {}})));
    CHECK(IsCenterPosition(
      TwoComponent({LengthPercentage<> {50_css_percentage}}, {LengthPercentage<> {50_css_percentage}})));
    CHECK(IsCenterPosition(TwoComponent({keywords::Center {}}, {LengthPercentage<> {50_css_percentage}})));

    CHECK_FALSE(IsCenterPosition(TwoComponent({keywords::Left {}}, {keywords::Center {}})));
    CHECK_FALSE(IsCenterPosition(TwoComponent({keywords::Center {}}, {LengthPercentage<> {50_css_px}})));
    CHECK_FALSE(
      IsCenterPosition(TwoComponent({LengthPercentage<> {25_css_percentage}}, {keywords::Center {}})));

    // Only the two-component form is checked.
    Position fourComponent {FourComponentPositionHorizontalVertical {
      FourComponentPositionHorizontal {{keywords::Left {}, LengthPercentage<> {50_css_percentage}}},
      FourComponentPositionVertical {{keywords::Top {}, LengthPercentage<> {50_css_percentage}}}}};
    CHECK_FALSE(IsCenterPosition(fourComponent));
  }

  TEST_CASE("Position(Split)", "[CSS][Primitives][Position]")
  {
    SECTION("two components")
    {
      auto [x, y] = Split(TwoComponent({keywords::Left {}}, {LengthPercentage<> {10_css_px}}));
      CHECK(x == PositionX {TwoComponentPositionHorizontal {keywords::Left {}}});
      CHECK(y == PositionY {TwoComponentPositionVertical {LengthPercentage<> {10_css_px}}});
    }

    SECTION("three components, length first")
    {
      FourComponentPositionHorizontal horizontal {{keywords::Right {}, LengthPercentage<> {5_css_px}}};
      Position position {ThreeComponentPositionHorizontalVerticalLengthFirst {
        horizontal, ThreeComponentPositionVertical {keywords::Bottom {}}}};

      auto [x, y] = Split(krys::move(position));
      CHECK(x == PositionX {horizontal});
      // The keyword-only half becomes its two-component form.
      CHECK(y == PositionY {TwoComponentPositionVertical {keywords::Bottom {}}});
    }

    SECTION("three components, length second")
    {
      FourComponentPositionVertical vertical {{keywords::Top {}, LengthPercentage<> {20_css_percentage}}};
      Position position {ThreeComponentPositionHorizontalVerticalLengthSecond {
        ThreeComponentPositionHorizontal {keywords::Center {}}, vertical}};

      auto [x, y] = Split(krys::move(position));
      CHECK(x == PositionX {TwoComponentPositionHorizontal {keywords::Center {}}});
      CHECK(y == PositionY {vertical});
    }

    SECTION("four components")
    {
      FourComponentPositionHorizontal horizontal {{keywords::XStart {}, LengthPercentage<> {1_css_px}}};
      FourComponentPositionVertical vertical {{keywords::YEnd {}, LengthPercentage<> {2_css_px}}};

      auto [x, y] = Split(Position {FourComponentPositionHorizontalVertical {horizontal, vertical}});
      CHECK(x == PositionX {horizontal});
      CHECK(y == PositionY {vertical});
    }
  }

  TEST_CASE("Position(equality)", "[CSS][Primitives][Position]")
  {
    CHECK(TwoComponent({keywords::Left {}}, {keywords::Top {}})
          == TwoComponent({keywords::Left {}}, {keywords::Top {}}));
    CHECK_FALSE(TwoComponent({keywords::Left {}}, {keywords::Top {}})
                == TwoComponent({keywords::Right {}}, {keywords::Top {}}));
  }

  TEST_CASE("Position(Serialise)", "[CSS][Primitives][Position]")
  {
    CHECK(SerialiseForTest(TwoComponent({keywords::Left {}}, {keywords::Top {}})) == u8"left top");
    CHECK(SerialiseForTest(
            TwoComponent({LengthPercentage<> {10_css_px}}, {LengthPercentage<> {50_css_percentage}}))
          == u8"10px 50%");
    CHECK(SerialiseForTest(Position {FourComponentPositionHorizontalVertical {
            FourComponentPositionHorizontal {{keywords::Right {}, LengthPercentage<> {5_css_px}}},
            FourComponentPositionVertical {{keywords::Bottom {}, LengthPercentage<> {10_css_percentage}}}}})
          == u8"right 5px bottom 10%");
    CHECK(SerialiseForTest(PositionX {TwoComponentPositionHorizontal {keywords::XStart {}}}) == u8"x-start");
    CHECK(SerialiseForTest(
            PositionY {FourComponentPositionVertical {{keywords::Top {}, LengthPercentage<> {1_css_em}}}})
          == u8"top 1em");
  }

  TEST_CASE("Position(dependencies and visitation)", "[CSS][Primitives][Position]")
  {
    auto position = TwoComponent({LengthPercentage<> {1_css_em}}, {keywords::Center {}});
    CHECK(CollectComputedStyleDependencies(position).Properties == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, position) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
