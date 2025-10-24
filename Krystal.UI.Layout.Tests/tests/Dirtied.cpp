#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI.Layout/Node/Node.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  static void _dirtied(NodeConstRef node)
  {
    int *dirtiedCount = (int *)NodeGetContext(node);
    (*dirtiedCount)++;
  }

  TEST_CASE("LayoutEngine::dirtied", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    int dirtiedCount = 0;
    NodeSetContext(root, &dirtiedCount);
    NodeSetDirtiedFunc(root, _dirtied);

    REQUIRE(0 == dirtiedCount);

    // `_dirtied` MUST be called in case of explicit dirtying.
    static_cast<NodeRef>(root)->SetDirty(true);
    REQUIRE(1 == dirtiedCount);

    // `_dirtied` MUST be called ONCE.
    static_cast<NodeRef>(root)->SetDirty(true);
    REQUIRE(1 == dirtiedCount);
  }

  TEST_CASE("LayoutEngine::dirtied_propagation", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    int dirtiedCount = 0;
    NodeSetContext(root, &dirtiedCount);
    NodeSetDirtiedFunc(root, _dirtied);

    REQUIRE(0 == dirtiedCount);

    // `_dirtied` MUST be called for the first time.
    static_cast<NodeRef>(root_child0)->MarkDirtyAndPropagate();
    REQUIRE(1 == dirtiedCount);

    // `_dirtied` must NOT be called for the second time.
    static_cast<NodeRef>(root_child0)->MarkDirtyAndPropagate();
    REQUIRE(1 == dirtiedCount);
  }

  TEST_CASE("LayoutEngine::dirtied_hierarchy", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    int dirtiedCount = 0;
    NodeSetContext(root_child0, &dirtiedCount);
    NodeSetDirtiedFunc(root_child0, _dirtied);

    REQUIRE(0 == dirtiedCount);

    // `_dirtied` must NOT be called for descendants.
    static_cast<NodeRef>(root)->MarkDirtyAndPropagate();
    REQUIRE(0 == dirtiedCount);

    // `_dirtied` must NOT be called for the sibling node.
    static_cast<NodeRef>(root_child1)->MarkDirtyAndPropagate();
    REQUIRE(0 == dirtiedCount);

    // `_dirtied` MUST be called in case of explicit dirtying.
    static_cast<NodeRef>(root_child0)->MarkDirtyAndPropagate();
    REQUIRE(1 == dirtiedCount);
  }
}