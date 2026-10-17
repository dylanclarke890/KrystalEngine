#include "Krystal.Booey/DOM/DOMRectList.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>

namespace krys::boo::dom::tests
{
  TEST_CASE("DOMRect: edges", "[DOM][DOMRect]")
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
    CHECK(fromInit->ToFloatRect() == FloatRect(1, 2, 3, 4));
  }

  TEST_CASE("DOMRectList: items", "[DOM][DOMRectList]")
  {
    CHECK(DOMRectList::Create()->Length() == 0u);

    // A list of quads holds their bounding boxes.
    auto list = DOMRectList::Create(
      List<FloatQuad> {FloatQuad(FloatRect(1, 2, 3, 4)), FloatQuad(FloatPoint(0, 0), FloatPoint(10, 5),
                                                                   FloatPoint(5, 20), FloatPoint(-5, 10))});
    REQUIRE(list->Length() == 2u);
    CHECK(list->Item(0)->ToFloatRect() == FloatRect(1, 2, 3, 4));
    CHECK(list->Item(1)->ToFloatRect() == FloatRect(-5, 0, 15, 20));
    CHECK(list->IsSupportedPropertyIndex(1));
    CHECK_FALSE(list->IsSupportedPropertyIndex(2));
    CHECK(list->Item(2) == nullptr);
  }
}
