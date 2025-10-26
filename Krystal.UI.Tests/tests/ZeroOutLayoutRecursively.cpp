#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::zero_out_layout", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef child = NodeCreate();
    NodeInsertChild(root, child, 0);
    NodeStyleSetWidth(child, 100);
    NodeStyleSetHeight(child, 100);
    NodeStyleSetMargin(child, Edge::Top, 10);
    NodeStyleSetPadding(child, Edge::Top, 10);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetMargin(child, Edge::Top));
    REQUIRE(10 == NodeLayoutGetPadding(child, Edge::Top));

    NodeStyleSetDisplay(child, Display::None);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetMargin(child, Edge::Top));
    REQUIRE(0 == NodeLayoutGetPadding(child, Edge::Top));

    NodeDestroyRecursive(root);
  }
}