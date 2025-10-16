#include "../utils/TestUtil.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::aspect_ratio_does_not_stretch_cross_axis_dim", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetOverflow(root_child0, Overflow::Scroll);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child0, 2);
    NodeStyleSetFlexShrink(root_child0_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child0, 0);
    NodeStyleSetAspectRatio(root_child0_child0_child0, 1 / 1);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child1, 5);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child2, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child2, 0);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child2_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child2_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child2_child0, 0);
    NodeStyleSetAspectRatio(root_child0_child0_child2_child0, 1 / 1);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);

    NodeRef root_child0_child0_child2_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child2_child0_child0, 5);
    NodeInsertChild(root_child0_child0_child2_child0, root_child0_child0_child2_child0_child0, 0);

    NodeRef root_child0_child0_child2_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child2_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child2_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child2_child0_child1, 0);
    NodeStyleSetAspectRatio(root_child0_child0_child2_child0_child1, 1 / 1);
    NodeInsertChild(root_child0_child0_child2_child0, root_child0_child0_child2_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(197 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(197 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(202 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0_child0));
    REQUIRE(5 == NodeLayoutGetWidth(root_child0_child0_child2_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child2_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0_child1));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2_child0_child1));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(103 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(197 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(98 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    REQUIRE(93 == NodeLayoutGetLeft(root_child0_child0_child2_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0_child0));
    REQUIRE(5 == NodeLayoutGetWidth(root_child0_child0_child2_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child2_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child2_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child2_child0_child1));
    REQUIRE(98 == NodeLayoutGetWidth(root_child0_child0_child2_child0_child1));
    REQUIRE(197 == NodeLayoutGetHeight(root_child0_child0_child2_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::zero_aspect_ratio_behaves_like_auto", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 0 / 1);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(250 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
