#include "Krystal.Web/Css/Geometry/DOMRectList.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>

namespace krys::web::tests
{
  TEST_CASE(
    "DOMRect: edges",
    "[CSS][Geometry][DOMRect][geometry-1:DOMRect][geometry-1:dom-domrectreadonly-domrect-top]"
    "[geometry-1:dom-domrectreadonly-domrect-right][geometry-1:dom-domrectreadonly-domrect-bottom]"
    "[geometry-1:dom-domrectreadonly-domrect-left][geometry-1:create-a-domrectreadonly-from-the-dictionary]"
    "[geometry-1:dom-domrectreadonly-domrectreadonly][geometry-1:dom-domrectreadonly-fromrect]"
    "[geometry-1:dom-domrect-fromrect][geometry-1:dom-domrectreadonly-domrect-x]"
    "[geometry-1:dom-domrectreadonly-domrect-y][geometry-1:dom-domrectreadonly-domrect-width]"
    "[geometry-1:dom-domrectreadonly-domrect-height]")
  {
    auto rect = DOMRect::Create(10, 20, 30, 40);
    CHECK(rect->Left() == 10);
    CHECK(rect->Top() == 20);
    CHECK(rect->Right() == 40);
    CHECK(rect->Bottom() == 60);

    // Negative sizes swap the edges.
    rect->SetWidth(-30);
    rect->SetHeight(-40);
    CHECK(rect->Left() == -20);
    CHECK(rect->Right() == 10);
    CHECK(rect->Top() == -20);
    CHECK(rect->Bottom() == 20);

    // NaN propagates, as with Math.min and Math.max.
    rect->SetX(std::nan(""));
    CHECK(std::isnan(rect->Left()));
    CHECK(std::isnan(rect->Right()));
    CHECK(rect->Top() == -20);

    auto fromInit = DOMRectReadOnly::FromRect({1, 2, 3, 4});
    CHECK(fromInit->ToRectF() == RectF(1, 2, 3, 4));
    auto mutableFromInit = DOMRect::FromRect({.Width = 5});
    mutableFromInit->SetY(6);
    CHECK(mutableFromInit->ToRectF() == RectF(0, 6, 5, 0));

    // The constructors' arguments default to 0.
    CHECK(DOMRectReadOnly::Create()->ToRectF() == RectF());
    CHECK(DOMRect::Create(1)->ToRectF() == RectF(1, 0, 0, 0));
  }

  TEST_CASE(
    "DOMRectList: items",
    "[CSS][Geometry][DOMRectList][geometry-1:dom-domrectlist-length][geometry-1:dom-domrectlist-item]")
  {
    CHECK(DOMRectList::Create()->Length() == 0u);

    // A list of quads holds their bounding boxes.
    auto list = DOMRectList::Create(List<QuadF> {
      QuadF(RectF(1, 2, 3, 4)), QuadF(PointF(0, 0), PointF(10, 5), PointF(5, 20), PointF(-5, 10))});
    REQUIRE(list->Length() == 2u);
    CHECK(list->Item(0)->ToRectF() == RectF(1, 2, 3, 4));
    CHECK(list->Item(1)->ToRectF() == RectF(-5, 0, 15, 20));
    CHECK(list->IsSupportedPropertyIndex(1));
    CHECK_FALSE(list->IsSupportedPropertyIndex(2));
    CHECK(list->Item(2) == nullptr);
  }
}
