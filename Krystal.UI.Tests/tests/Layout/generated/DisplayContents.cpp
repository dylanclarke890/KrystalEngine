#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::test1", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
