#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;
  static float _baseline(NodeConstRef node, const float /*width*/, const float /*height*/)
  {
    auto *baseline = (float *)NodeGetContext(node);
    return *baseline;
  }

  TEST_CASE("LayoutEngine::align_baseline_customer_func", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    float baselineValue = 10;
    NodeRef root_child1_child0 = NodeCreate();
    NodeSetContext(root_child1_child0, &baselineValue);
    NodeStyleSetWidth(root_child1_child0, 50);
    NodeSetBaselineFunc(root_child1_child0, _baseline);
    NodeStyleSetHeight(root_child1_child0, 20);
    NodeInsertChild(root_child1, root_child1_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1_child0));
  }
}