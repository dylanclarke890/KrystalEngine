#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include "Krystal.UI/Styles/Enums/Gutter.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("FlexGap::gap_negative_value", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetGap(root, Gutter::All, -20);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(200 == NodeLayoutGetHeight(root_child2));

    REQUIRE(60 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(200 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(200 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(200 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}