#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI.Layout/Style/Style.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;

  TEST_CASE("Style - Computed Padding Is Floored", "[Style]")
  {
    Style style;
    style.SetPadding(Edge::All, StyleLength::Points(-1.0f));
    auto paddingStart =
      style.ComputeInlineStartPadding(FlexDirection::Row, Direction::LTR, 0.0f /*widthSize*/);
    REQUIRE(paddingStart == 0.0f);
  }

  TEST_CASE("Style - Computed Border Is Floored", "[Style]")
  {
    Style style;
    style.SetBorder(Edge::All, StyleLength::Points(-1.0f));
    auto borderStart = style.ComputeInlineStartBorder(FlexDirection::Row, Direction::LTR);
    REQUIRE(borderStart == 0.0f);
  }

  TEST_CASE("Style - Computed Gap Is Floored", "[Style]")
  {
    Style style;
    style.SetGap(Gutter::Column, StyleLength::Points(-1.0f));
    auto gapBetweenColumns = style.ComputeGapForAxis(FlexDirection::Row, 0.0);
    REQUIRE(gapBetweenColumns == 0.0f);
  }

  TEST_CASE("Style - Computed Margin Is Not Floored", "[Style]")
  {
    Style style;
    style.SetMargin(Edge::All, StyleLength::Points(-1.0f));
    auto marginStart = style.ComputeInlineStartMargin(FlexDirection::Row, Direction::LTR, 0.0f /*widthSize*/);
    REQUIRE(marginStart == -1.0f);
  }

}