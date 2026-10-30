#include "Krystal.UI/PathStrokerStrategy.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::ui::tests
{
  using Catch::Approx;

  TEST_CASE("PathStrokerStrategy: strokes as Render2D draws them", "[UI]")
  {
    PathStrokerStrategy strategy;

    boo::Path line;
    line.MoveTo({10, 10});
    line.AddLineTo({50, 10});
    boo::PathStrokeStyle style {.Thickness = 4};

    // Butt caps stop at the ends.
    CHECK(strategy.StrokeContains(line, {30, 11}, style));
    CHECK_FALSE(strategy.StrokeContains(line, {30, 13}, style));
    CHECK_FALSE(strategy.StrokeContains(line, {8, 10}, style));
    auto bounds = strategy.StrokeBoundingRect(line, style);
    CHECK(bounds.X() == Approx(10));
    CHECK(bounds.Y() == Approx(8));
    CHECK(bounds.Width() == Approx(40));
    CHECK(bounds.Height() == Approx(4));

    // Square caps go on for half the width.
    style.Cap = boo::LineCap::Square;
    CHECK(strategy.StrokeContains(line, {8.5f, 10}, style));
    CHECK(strategy.StrokeBoundingRect(line, style).X() == Approx(8));

    // Gaps between dashes aren't inside the stroke.
    style.Cap = boo::LineCap::Butt;
    style.Dashes = boo::DashArray {5, 5};
    CHECK(strategy.StrokeContains(line, {12, 10}, style));
    CHECK_FALSE(strategy.StrokeContains(line, {17, 10}, style));

    // A miter join's point is inside the stroke and its bounds; a bevel's isn't.
    boo::Path corner;
    corner.MoveTo({0, 20});
    corner.AddLineTo({20, 0});
    corner.AddLineTo({40, 20});
    boo::PathStrokeStyle cornerStyle {.Thickness = 4, .Join = boo::LineJoin::Miter};
    CHECK(strategy.StrokeContains(corner, {20, -2.5f}, cornerStyle));
    CHECK(strategy.StrokeBoundingRect(corner, cornerStyle).Y() < -2.5f);
    cornerStyle.Join = boo::LineJoin::Bevel;
    CHECK_FALSE(strategy.StrokeContains(corner, {20, -2.5f}, cornerStyle));

    // An empty path has no stroke.
    CHECK(strategy.StrokeBoundingRect(boo::Path(), style) == boo::FloatRect());
  }
}
