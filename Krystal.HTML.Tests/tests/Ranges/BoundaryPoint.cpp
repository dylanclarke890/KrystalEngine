#include "Krystal.HTML/Ranges/BoundaryPoint.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("BoundaryPoint::ComparePositionTo", "[HTML][BoundaryPoint]")
  {
    auto document = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*document);
    auto childA = CreateRef<TestContainerNode>(*document);
    auto childB = CreateRef<TestContainerNode>(*document);

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());

    BoundaryPoint pointA {childA, 0};
    BoundaryPoint pointB {childB, 0};

    REQUIRE(pointA.ComparePositionTo(pointB) == std::strong_ordering::less);
    REQUIRE(pointB.ComparePositionTo(pointA) == std::strong_ordering::greater);
    REQUIRE(pointA.ComparePositionTo(pointA) == std::strong_ordering::equal);

    BoundaryPoint pointAOffset {childA, 1};
    BoundaryPoint pointBOffset {childB, 1};

    REQUIRE(pointAOffset.ComparePositionTo(pointBOffset) == std::strong_ordering::less);
    REQUIRE(pointBOffset.ComparePositionTo(pointAOffset) == std::strong_ordering::greater);
    REQUIRE(pointAOffset.ComparePositionTo(pointAOffset) == std::strong_ordering::equal);

    BoundaryPoint pointAParent {parent, 0};
    BoundaryPoint pointBChild {childB, 0};

    REQUIRE(pointAParent.ComparePositionTo(pointBChild) == std::strong_ordering::less);
    REQUIRE(pointBChild.ComparePositionTo(pointAParent) == std::strong_ordering::greater);
    REQUIRE(pointAParent.ComparePositionTo(pointAParent) == std::strong_ordering::equal);

    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
  }
}