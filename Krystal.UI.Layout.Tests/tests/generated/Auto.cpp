#include "../utils/TestUtil.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  TEST_CASE("LayoutEngine::auto_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidthAuto(root);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::auto_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeightAuto(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::auto_flex_basis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::auto_position", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionAuto(root_child0, Edge::Right);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::auto_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMarginAuto(root_child0, Edge::Left);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
