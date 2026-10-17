#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::layout::tests
{
  namespace
  {
    KRYS_NODISCARD BoxGeometry MakeGeometry() noexcept
    {
      // A 100x50 content box at (10,20), with margins 1/2/3/4, borders 5/6/7/8 and padding 9/10/11/12 (start,
      // end, before, after), and 15px for a vertical scrollbar.
      auto geometry = BoxGeometry {};
      geometry.SetTopLeft({10_lu, 20_lu});
      geometry.SetContentBoxSize({100_lu, 50_lu});
      geometry.SetHorizontalMargin({1_lu, 2_lu});
      geometry.SetVerticalMargin({3_lu, 4_lu});
      geometry.SetBorder({{5_lu, 6_lu}, {7_lu, 8_lu}});
      geometry.SetPadding({{9_lu, 10_lu}, {11_lu, 12_lu}});
      geometry.SetHorizontalSpaceForScrollbar(15_lu);
      return geometry;
    }

    KRYS_NODISCARD LayoutRect ToRect(const Rect &rect) noexcept
    {
      return rect;
    }
  }

  TEST_CASE("BoxGeometry: boxes and edges", "[Layout][LayoutTree][BoxGeometry]")
  {
    auto geometry = MakeGeometry();

    CHECK(geometry.HorizontalBorder() == 11_lu);
    CHECK(geometry.VerticalBorder() == 15_lu);
    CHECK(geometry.HorizontalPadding() == 19_lu);
    CHECK(geometry.VerticalPadding() == 23_lu);
    CHECK(geometry.BorderAndPaddingStart() == 14_lu);
    CHECK(geometry.BorderAndPaddingAfter() == 20_lu);
    CHECK(geometry.HorizontalBorderAndPadding() == 30_lu);
    CHECK(geometry.VerticalBorderAndPadding() == 38_lu);
    CHECK(geometry.MarginBorderAndPaddingStart() == 15_lu);
    CHECK(geometry.HorizontalMarginBorderAndPadding() == 33_lu);
    CHECK(geometry.VerticalMarginBorderAndPadding() == 45_lu);
    CHECK(geometry.HasMarginBorderOrPadding());

    // The content box is inside the border and padding; the scrollbar sits between the padding box and the
    // end border.
    CHECK(geometry.ContentBoxTop() == 18_lu);
    CHECK(geometry.ContentBoxLeft() == 14_lu);
    CHECK(geometry.ContentBoxBottom() == 68_lu);
    CHECK(geometry.ContentBoxRight() == 114_lu);
    CHECK(geometry.PaddingBoxWidth() == 119_lu);
    CHECK(geometry.PaddingBoxHeight() == 73_lu);
    CHECK(geometry.PaddingBoxRight() == 124_lu);
    CHECK(geometry.BorderBoxWidth() == 145_lu);
    CHECK(geometry.BorderBoxHeight() == 88_lu);
    CHECK(geometry.MarginBoxWidth() == 148_lu);
    CHECK(geometry.MarginBoxHeight() == 95_lu);

    // These are relative to the border box.
    CHECK(ToRect(geometry.BorderBox()) == LayoutRect {0_lu, 0_lu, 145_lu, 88_lu});
    CHECK(ToRect(geometry.MarginBox()) == LayoutRect {-1_lu, -3_lu, 148_lu, 95_lu});
    CHECK(ToRect(geometry.PaddingBox()) == LayoutRect {5_lu, 7_lu, 119_lu, 73_lu});
    CHECK(ToRect(geometry.ContentBox()) == LayoutRect {14_lu, 18_lu, 100_lu, 50_lu});

    // And these to the containing block.
    CHECK(BoxGeometry::BorderBoxTopLeft(geometry) == LayoutPoint {10_lu, 20_lu});
    CHECK(ToRect(BoxGeometry::BorderBoxRect(geometry)) == LayoutRect {10_lu, 20_lu, 145_lu, 88_lu});
    CHECK(ToRect(BoxGeometry::MarginBoxRect(geometry)) == LayoutRect {9_lu, 17_lu, 148_lu, 95_lu});

    geometry.MoveHorizontally(5_lu);
    geometry.MoveVertically(-5_lu);
    CHECK(BoxGeometry::BorderBoxTopLeft(geometry) == LayoutPoint {15_lu, 15_lu});
    geometry.MoveBy({1_lu, 1_lu});
    geometry.Move({1_lu, 2_lu});
    CHECK(BoxGeometry::BorderBoxTopLeft(geometry) == LayoutPoint {17_lu, 18_lu});

    geometry.SetMarginStart(-4_lu);
    CHECK(geometry.HorizontalMargin().Start == -4_lu);
    CHECK(geometry.HorizontalMargin().End == 2_lu);
    geometry.SetMarginEnd(0_lu);
    CHECK(geometry.MarginEnd() == 0_lu);

    auto copy = geometry;
    copy.Reset();
    CHECK(BoxGeometry::BorderBoxTopLeft(copy) == LayoutPoint {});
    CHECK(copy.MarginBoxWidth() == 0_lu);
    CHECK(copy.BorderBoxHeight() == 0_lu);
    CHECK_FALSE(copy.HasMarginBorderOrPadding());
    CHECK(geometry.BorderBoxWidth() == 145_lu);
  }

  TEST_CASE("BoxGeometry: rects", "[Layout][LayoutTree][BoxGeometry]")
  {
    auto rect = Rect {10_lu, 20_lu, 30_lu, 40_lu};
    CHECK(rect.Top() == 10_lu);
    CHECK(rect.Left() == 20_lu);
    CHECK(rect.Right() == 50_lu);
    CHECK(rect.Bottom() == 50_lu);
    CHECK(rect.TopLeft() == LayoutPoint {20_lu, 10_lu});
    CHECK(rect.BottomRight() == LayoutPoint {50_lu, 50_lu});
    CHECK(rect.Size() == LayoutSize {30_lu, 40_lu});

    rect.ShiftLeftTo(25_lu);
    rect.ShiftBottomTo(60_lu);
    CHECK(LayoutRect(rect) == LayoutRect {25_lu, 10_lu, 25_lu, 50_lu});
    rect.ShiftRightTo(55_lu);
    rect.ShiftTopTo(0_lu);
    CHECK(LayoutRect(rect) == LayoutRect {25_lu, 0_lu, 30_lu, 60_lu});

    rect.MoveHorizontally(5_lu);
    rect.MoveVertically(5_lu);
    rect.ExpandHorizontally(10_lu);
    rect.ExpandVertically(-10_lu);
    CHECK(LayoutRect(rect) == LayoutRect {30_lu, 5_lu, 40_lu, 50_lu});

    rect.ExpandToContain(Rect {LayoutPoint {0_lu, 0_lu}, LayoutSize {10_lu, 10_lu}});
    CHECK(LayoutRect(rect) == LayoutRect {0_lu, 0_lu, 70_lu, 55_lu});
    CHECK(rect.Intersects(Rect {60_lu, 60_lu, 5_lu, 5_lu}) == false);
    CHECK(rect.Intersects(Rect {50_lu, 60_lu, 5_lu, 5_lu}));
    CHECK_FALSE(rect.IsEmpty());
    CHECK(LayoutRect(rect.Clone()) == LayoutRect(rect));
    CHECK(Rect(FloatRect {1.5f, 2.f, 3.f, 4.f}).Left() == LayoutUnit(1.5f));
  }
}
