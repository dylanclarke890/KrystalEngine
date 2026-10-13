#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    css::CustomIdentifier Ident(const char8 *name)
    {
      return css::CustomIdentifier {css::CSSOMStringAtom {name}};
    }

    css::CustomIdentifier NoName()
    {
      return css::CustomIdentifier {css::CSSOMStringAtom::Null()};
    }
  }

  TEST_CASE("GridPosition(Kinds)", "[Style][Grid][GridPosition]")
  {
    GridPosition automatic {css::keywords::Auto {}};
    CHECK(automatic.IsAuto());
    CHECK(automatic.ShouldBeResolvedAgainstOppositePosition());

    GridPosition explicitPosition {GridPosition::Explicit {{3}, Ident(u8"a")}};
    CHECK(explicitPosition.IsExplicit());
    CHECK(explicitPosition.ExplicitPosition() == 3);
    CHECK(explicitPosition.NamedGridLine() == u8"a");
    CHECK_FALSE(explicitPosition.ShouldBeResolvedAgainstOppositePosition());

    GridPosition span {GridPosition::Span {{2}, Ident(u8"b")}};
    CHECK(span.IsSpan());
    CHECK(span.SpanPosition() == 2);
    CHECK(span.NamedGridLine() == u8"b");
    CHECK(span.ShouldBeResolvedAgainstOppositePosition());

    GridPosition named {Ident(u8"c")};
    CHECK(named.IsNamedGridArea());
    CHECK(named.NamedGridLine() == u8"c");

    CHECK(explicitPosition == GridPosition {GridPosition::Explicit {{3}, Ident(u8"a")}});
    CHECK(explicitPosition != GridPosition {GridPosition::Span {{3}, Ident(u8"a")}});
    CHECK(automatic != named);
  }

  TEST_CASE("GridPosition(Limits)", "[Style][Grid][GridPosition]")
  {
    CHECK(GridPosition::Max() == 1'000'000);
    CHECK(GridPosition::Min() == -1'000'000);

    GridPosition::SetMaxPositionForTesting(5);
    CHECK(GridPosition::Max() == 5);
    CHECK(GridPosition::Min() == -5);
    CHECK(GridPosition {GridPosition::Explicit {{9}, NoName()}}.ExplicitPosition() == 5);
    CHECK(GridPosition {GridPosition::Explicit {{-9}, NoName()}}.ExplicitPosition() == -5);
    CHECK(GridPosition {GridPosition::Span {{9}, NoName()}}.SpanPosition() == 5);
    GridPosition::SetMaxPositionForTesting(1'000'000);
  }

  TEST_CASE("GridPosition(SwitchOn)", "[Style][Grid][GridPosition]")
  {
    auto kind = [](const GridPosition &position)
    {
      return position.SwitchOn([](css::keywords::Auto) { return 0; }, [](const GridPosition::Explicit &)
                               { return 1; }, [](const GridPosition::Span &) { return 2; },
                               [](const css::CustomIdentifier &) { return 3; });
    };

    CHECK(kind(GridPosition {css::keywords::Auto {}}) == 0);
    CHECK(kind(GridPosition {GridPosition::Explicit {{1}, NoName()}}) == 1);
    CHECK(kind(GridPosition {GridPosition::Span {{1}, NoName()}}) == 2);
    CHECK(kind(GridPosition {Ident(u8"a")}) == 3);

    // The parts round-trip through SwitchOn.
    GridPosition::Span span {{4}, Ident(u8"x")};
    GridPosition {span}.SwitchOn([&](const GridPosition::Span &value) { CHECK(value == span); },
                                 [](const auto &) { FAIL(); });

    // `Explicit` without a name is just its integer; `Span` with an integer of 1 drops it.
    auto explicitParts = GridPosition::Explicit {{7}, NoName()}.SwitchOn(
      [](const GridPosition::Explicit::PositionType &) { return 1; }, [](const auto &) { return 2; });
    CHECK(explicitParts == 1);

    auto spanParts = GridPosition::Span {{1}, Ident(u8"x")}.SwitchOn(
      [](const css::SpaceSeparatedTuple<css::keywords::Span, css::CustomIdentifier> &) { return 1; },
      [](const auto &) { return 2; });
    CHECK(spanParts == 1);
  }
}
