#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;
  static void RecursivelyAssertProperNodeOwnership(NodeRef node)
  {
    for (size_t i = 0; i < NodeGetChildCount(node); ++i)
    {
      const auto child = NodeGetChild(node, i);
      REQUIRE(node == NodeGetOwner(child));
      RecursivelyAssertProperNodeOwnership(child);
    }
  }

  TEST_CASE("LayoutEngine::absolute_node_cloned_with_static_parent", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0, 1);
    NodeStyleSetHeight(root_child0_child0, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeRef clonedRoot = NodeClone(root);
    NodeStyleSetWidth(clonedRoot, 110);
    NodeCalculateLayout(clonedRoot, Undefined, Undefined, Direction::LTR);

    RecursivelyAssertProperNodeOwnership(clonedRoot);

    NodeDestroyRecursive(root);
    NodeDestroyRecursive(clonedRoot);
  }

  TEST_CASE("LayoutEngine::absolute_node_cloned_with_static_ancestors", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 40);
    NodeStyleSetHeight(root_child0_child0, 40);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0_child0, 30);
    NodeStyleSetHeight(root_child0_child0_child0, 30);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0_child0, 1);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 1);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeRef clonedRoot = NodeClone(root);
    NodeStyleSetWidth(clonedRoot, 110);
    NodeCalculateLayout(clonedRoot, Undefined, Undefined, Direction::LTR);

    RecursivelyAssertProperNodeOwnership(clonedRoot);

    NodeDestroyRecursive(root);
    NodeDestroyRecursive(clonedRoot);
  }
}