#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("LayoutRect: construction", "[Graphics][LayoutRect]")
  {
    LayoutRect rect(1, 2, 3, 4);
    CHECK(rect.X() == 1);
    CHECK(rect.Y() == 2);
    CHECK(rect.Width() == 3);
    CHECK(rect.Height() == 4);
    CHECK(rect.MaxX() == 4);
    CHECK(rect.MaxY() == 6);
    CHECK_FALSE(rect.IsEmpty());
    CHECK(LayoutRect().IsEmpty());

    CHECK(LayoutRect(LayoutPoint(1, 2), LayoutPoint(4, 6)) == rect);
    CHECK(LayoutRect(IntRect(1, 2, 3, 4)) == rect);
    CHECK(LayoutRect(FloatPoint(1, 2), FloatSize(3, 4)) == rect);
    CHECK(LayoutRect(FloatRect(1.5f, 2.25f, 3, 4)) == LayoutRect(LayoutUnit(1.5f), LayoutUnit(2.25f), 3, 4));

    FloatRect asFloat = LayoutRect(LayoutUnit(1.5f), 2, 3, 4);
    CHECK(asFloat == FloatRect(1.5f, 2, 3, 4));
    CHECK(IntRect(LayoutRect(LayoutUnit(1.5f), 2, 3, 4)) == IntRect(1, 2, 3, 4));

    CHECK(rect.Center() == LayoutPoint(LayoutUnit(2.5f), 4));
    CHECK(rect.TransposedRect() == LayoutRect(2, 1, 4, 3));
  }

  TEST_CASE("LayoutRect: move, expand and shift", "[Graphics][LayoutRect]")
  {
    LayoutRect rect(10, 20, 100, 200);
    rect.Move(LayoutSize(5, 5));
    rect.MoveBy(LayoutPoint(-5, -5));
    rect.Move(1, 1);
    CHECK(rect.Location() == LayoutPoint(11, 21));

    rect.Expand(LayoutSize(10, 10));
    rect.Expand(-10, -10);
    rect.Contract(LayoutSize(50, 100));
    rect.Contract(-50, -100);
    CHECK(rect.Size() == LayoutSize(100, 200));

    LayoutBoxExtent box(LayoutUnit(1), LayoutUnit(2), LayoutUnit(3), LayoutUnit(4));
    rect.Expand(box);
    CHECK(rect == LayoutRect(7, 20, 106, 204));
    rect.Contract(box);
    CHECK(rect == LayoutRect(11, 21, 100, 200));

    rect.ShiftXEdgeTo(21);
    rect.ShiftYEdgeTo(31);
    CHECK(rect == LayoutRect(21, 31, 90, 190));
    rect.ShiftMaxXEdgeTo(100);
    rect.ShiftMaxYEdgeTo(20);
    CHECK(rect == LayoutRect(21, 31, 79, 0));
    rect.ShiftXEdgeBy(LayoutUnit(1));
    rect.ShiftYEdgeBy(LayoutUnit(1));
    rect.ShiftMaxXEdgeBy(LayoutUnit(2));
    rect.ShiftMaxYEdgeBy(LayoutUnit(5));
    CHECK(rect == LayoutRect(22, 32, 80, 5));

    rect.Inflate(2);
    CHECK(rect == LayoutRect(20, 30, 84, 9));
    rect.InflateX(LayoutUnit(1));
    rect.InflateY(1);
    rect.Inflate(LayoutSize(1, 1));
    CHECK(rect == LayoutRect(18, 28, 88, 13));

    rect.ShiftEdgesTo(LayoutUnit(1), LayoutUnit(2), LayoutUnit(5), LayoutUnit(10));
    CHECK(rect == LayoutRect(1, 2, 4, 8));
  }

  TEST_CASE("LayoutRect: containment and intersection", "[Graphics][LayoutRect]")
  {
    LayoutRect rect(20, 30, 100, 200);
    CHECK(rect.Contains(LayoutRect(30, 40, 50, 100)));
    CHECK_FALSE(rect.Contains(LayoutRect(10, 20, 90, 180)));
    CHECK(rect.Contains(LayoutPoint(20, 30)));
    CHECK_FALSE(rect.Contains(LayoutPoint(120, 30)));

    CHECK(rect.Intersects(LayoutRect(10, 20, 90, 180)));
    CHECK_FALSE(rect.Intersects(LayoutRect(120, 230, 50, 100)));

    CHECK(Intersection(rect, LayoutRect(70, 80, 100, 200)) == LayoutRect(70, 80, 50, 150));
    CHECK(Intersection(rect, LayoutRect(200, 300, 10, 10)) == LayoutRect());

    LayoutRect edge(0, 0, 10, 10);
    CHECK(edge.EdgeInclusiveIntersect(LayoutRect(10, 0, 10, 10)));
    CHECK(edge == LayoutRect(10, 0, 0, 10));
    CHECK_FALSE(edge.EdgeInclusiveIntersect(LayoutRect(20, 20, 1, 1)));
    CHECK(edge == LayoutRect());
  }

  TEST_CASE("LayoutRect: union", "[Graphics][LayoutRect]")
  {
    CHECK(UnionRect(LayoutRect(20, 30, 100, 200), LayoutRect(70, 80, 100, 200))
          == LayoutRect(20, 30, 150, 250));

    LayoutRect rect(10, 10, 10, 10);
    rect.Unite(LayoutRect(100, 100, 0, 5));
    CHECK(rect == LayoutRect(10, 10, 10, 10));
    rect.UniteEvenIfEmpty(LayoutRect(100, 100, 0, 5));
    CHECK(rect == LayoutRect(10, 10, 90, 95));

    LayoutRect nonZero(10, 10, 10, 10);
    nonZero.UniteIfNonZero(LayoutRect(0, 0, 0, 0));
    CHECK(nonZero == LayoutRect(10, 10, 10, 10));
    nonZero.UniteIfNonZero(LayoutRect(0, 0, 0, 5));
    CHECK(nonZero == LayoutRect(0, 0, 20, 20));

    LayoutRect rects[] = {LayoutRect(0, 0, 1, 1), LayoutRect(5, 5, 1, 1), LayoutRect(2, 10, 1, 1)};
    CHECK(UnionRect(Span<const LayoutRect>(rects)) == LayoutRect(0, 0, 6, 11));
    CHECK(UnionRect(Span<const LayoutRect>()) == LayoutRect());

    LayoutRect checked(0, 0, 10, 10);
    CHECK(checked.CheckedUnite(LayoutRect(20, 20, 10, 10)));
    CHECK(checked == LayoutRect(0, 0, 30, 30));
    // The union would be wider than a LayoutUnit can hold.
    LayoutRect wide(LayoutUnit::NearlyMin() / 2, 0, 10, 10);
    CHECK_FALSE(wide.CheckedUnite(LayoutRect(LayoutUnit::NearlyMax() / 2, 0, 10, 10)));
    CHECK(wide == LayoutRect(LayoutUnit::NearlyMin() / 2, 0, 10, 10));
    CHECK(LayoutRect(0, 0, 1, 1).IsMaxXMaxYRepresentable());
    CHECK_FALSE(LayoutRect(LayoutUnit::NearlyMax(), 0, 1, 1).IsMaxXMaxYRepresentable());
  }

  TEST_CASE("LayoutRect: infinite rects and scaling", "[Graphics][LayoutRect]")
  {
    CHECK(LayoutRect::InfiniteRect().IsInfinite());
    CHECK_FALSE(LayoutRect(0, 0, 1, 1).IsInfinite());

    LayoutRect infinite = LayoutRect::InfiniteRect();
    infinite.Scale(2);
    CHECK(infinite.IsInfinite());

    LayoutRect rect(10, 20, 30, 40);
    rect.Scale(2);
    CHECK(rect == LayoutRect(20, 40, 60, 80));
    rect.Scale(0.5f, 0.25f);
    CHECK(rect == LayoutRect(10, 10, 30, 20));

    rect.ExpandToInfiniteX();
    CHECK(rect.X() == LayoutRect::InfiniteRect().X());
    CHECK(rect.Width() == LayoutRect::InfiniteRect().Width());
    CHECK(rect.Y() == 10);
    rect.ExpandToInfiniteY();
    CHECK(rect.IsInfinite());
  }

  TEST_CASE("LayoutRect: snapping and enclosing", "[Graphics][LayoutRect]")
  {
    LayoutRect rect(LayoutUnit(1.5f), LayoutUnit(2.25f), LayoutUnit(10.5f), LayoutUnit(3.5f));

    CHECK(SnappedIntRect(rect) == IntRect(2, 2, 10, 4));
    CHECK(SnappedIntRect(rect.X(), rect.Y(), rect.Width(), rect.Height()) == SnappedIntRect(rect));
    CHECK(SnappedIntRect(rect.Location(), rect.Size()) == SnappedIntRect(rect));

    CHECK(EnclosingIntRect(rect) == IntRect(1, 2, 11, 4));
    // Empty rects stay empty.
    CHECK(EnclosingIntRect(LayoutRect(LayoutUnit(1.5f), LayoutUnit(2.5f), 0, 3)) == IntRect(1, 2, 0, 4));

    CHECK(EnclosingLayoutRect(FloatRect(1.01f, 2, 3, 4))
          == LayoutRect(LayoutUnit(1), 2, LayoutUnit(3.015625f), 4));

    CHECK(SnapRectToDevicePixels(rect, 1) == FloatRect(2, 2, 10, 4));
    CHECK(SnapRectToDevicePixels(rect, 2) == FloatRect(1.5f, 2.5f, 10.5f, 3.5f));
    CHECK(SnapRectToDevicePixels(rect.X(), rect.Y(), rect.Width(), rect.Height(), 2)
          == SnapRectToDevicePixels(rect, 2));
    CHECK(SnapRectToDevicePixelsWithWritingDirection(rect, 2, true) == SnapRectToDevicePixels(rect, 2));
    CHECK(SnapRectToDevicePixelsWithWritingDirection(rect, 1, false) == FloatRect(1, 2, 11, 4));

    CHECK(EncloseRectToDevicePixels(rect, 1) == FloatRect(1, 2, 11, 4));
    CHECK(EncloseRectToDevicePixels(rect, 2) == FloatRect(1.5f, 2, 10.5f, 4));
  }
}
