#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "utils/TestUtil.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::cloning_shared_root", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(75 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeRef root2 = NodeClone(root);
    NodeStyleSetWidth(root2, 100);

    REQUIRE(2u == NodeGetChildCount(root2));
    // The children should have referential equality at this point.
    REQUIRE(root_child0 == NodeGetChild(root2, 0));
    REQUIRE(root_child1 == NodeGetChild(root2, 1));

    NodeCalculateLayout(root2, Undefined, Undefined, Direction::LTR);

    REQUIRE(2u == NodeGetChildCount(root2));
    // Relayout with no changed input should result in referential equality.
    REQUIRE(root_child0 == NodeGetChild(root2, 0));
    REQUIRE(root_child1 == NodeGetChild(root2, 1));

    NodeStyleSetWidth(root2, 150);
    NodeStyleSetHeight(root2, 200);
    NodeCalculateLayout(root2, Undefined, Undefined, Direction::LTR);

    REQUIRE(2u == NodeGetChildCount(root2));
    // Relayout with changed input should result in cloned children.
    NodeRef root2_child0 = NodeGetChild(root2, 0);
    NodeRef root2_child1 = NodeGetChild(root2, 1);
    REQUIRE(root_child0 != root2_child0);
    REQUIRE(root_child1 != root2_child1);

    // Everything in the root should remain unchanged.
    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(75 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    // The new root now has new layout.
    REQUIRE(0 == NodeLayoutGetLeft(root2));
    REQUIRE(0 == NodeLayoutGetTop(root2));
    REQUIRE(150 == NodeLayoutGetWidth(root2));
    REQUIRE(200 == NodeLayoutGetHeight(root2));

    REQUIRE(0 == NodeLayoutGetLeft(root2_child0));
    REQUIRE(0 == NodeLayoutGetTop(root2_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root2_child0));
    REQUIRE(125 == NodeLayoutGetHeight(root2_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root2_child1));
    REQUIRE(125 == NodeLayoutGetTop(root2_child1));
    REQUIRE(150 == NodeLayoutGetWidth(root2_child1));
    REQUIRE(75 == NodeLayoutGetHeight(root2_child1));

    NodeDestroyRecursive(root2);

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::mutating_children_of_a_clone_clones_only_after_layout", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    REQUIRE(0u == NodeGetChildCount(root));

    NodeRef root2 = NodeClone(root);
    REQUIRE(0u == NodeGetChildCount(root2));

    NodeRef root2_child0 = NodeCreate(config);
    NodeInsertChild(root2, root2_child0, 0);

    REQUIRE(0u == NodeGetChildCount(root));
    REQUIRE(1u == NodeGetChildCount(root2));

    NodeRef root3 = NodeClone(root2);
    REQUIRE(1u == NodeGetChildCount(root2));
    REQUIRE(1u == NodeGetChildCount(root3));
    REQUIRE(NodeGetChild(root2, 0) == NodeGetChild(root3, 0));

    NodeRef root3_child1 = NodeCreate(config);
    NodeInsertChild(root3, root3_child1, 1);
    REQUIRE(1u == NodeGetChildCount(root2));
    REQUIRE(2u == NodeGetChildCount(root3));
    REQUIRE(root3_child1 == NodeGetChild(root3, 1));
    REQUIRE(NodeGetChild(root2, 0) == NodeGetChild(root3, 0));

    NodeRef root4 = NodeClone(root3);
    REQUIRE(root3_child1 == NodeGetChild(root4, 1));

    NodeRemoveChild(root4, root3_child1);
    REQUIRE(2u == NodeGetChildCount(root3));
    REQUIRE(1u == NodeGetChildCount(root4));
    REQUIRE(NodeGetChild(root3, 0) == NodeGetChild(root4, 0));

    NodeCalculateLayout(root4, Undefined, Undefined, Direction::LTR);
    REQUIRE(NodeGetChild(root3, 0) != NodeGetChild(root4, 0));
    NodeCalculateLayout(root3, Undefined, Undefined, Direction::LTR);
    REQUIRE(NodeGetChild(root2, 0) != NodeGetChild(root3, 0));

    NodeDestroyRecursive(root4);
    NodeDestroyRecursive(root3);
    NodeDestroyRecursive(root2);
    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::cloning_two_levels", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexBasis(root_child0, 15);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child1_0, 10);
    NodeStyleSetFlexGrow(root_child1_0, 1);
    NodeInsertChild(root_child1, root_child1_0, 0);

    NodeRef root_child1_1 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child1_1, 25);
    NodeInsertChild(root_child1, root_child1_1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(40 == NodeLayoutGetHeight(root_child0));
    REQUIRE(60 == NodeLayoutGetHeight(root_child1));
    REQUIRE(35 == NodeLayoutGetHeight(root_child1_0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1_1));

    NodeRef root2_child0 = NodeClone(root_child0);
    NodeRef root2_child1 = NodeClone(root_child1);
    NodeRef root2 = NodeClone(root);

    NodeStyleSetFlexGrow(root2_child0, 0);
    NodeStyleSetFlexBasis(root2_child0, 40);

    NodeRemoveAllChildren(root2);
    NodeInsertChild(root2, root2_child0, 0);
    NodeInsertChild(root2, root2_child1, 1);
    REQUIRE(2u == NodeGetChildCount(root2));

    NodeCalculateLayout(root2, Undefined, Undefined, Direction::LTR);

    // Original root is unchanged
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));
    REQUIRE(60 == NodeLayoutGetHeight(root_child1));
    REQUIRE(35 == NodeLayoutGetHeight(root_child1_0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1_1));

    // New root has new layout at the top
    REQUIRE(40 == NodeLayoutGetHeight(root2_child0));
    REQUIRE(60 == NodeLayoutGetHeight(root2_child1));

    // The deeper children are untouched.
    REQUIRE(NodeGetChild(root2_child1, 0) == root_child1_0);
    REQUIRE(NodeGetChild(root2_child1, 1) == root_child1_1);

    NodeDestroyRecursive(root2);
    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::cloning_and_freeing", "[LayoutEngine]")
  {
    TestUtil::StartCountingNodes();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeRef root_child0 = NodeCreate(config);
    NodeInsertChild(root, root_child0, 0);
    NodeRef root_child1 = NodeCreate(config);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeRef root2 = NodeClone(root);

    // Freeing the original root should be safe as long as we don't free its
    // children.
    NodeDestroy(root);

    NodeCalculateLayout(root2, Undefined, Undefined, Direction::LTR);

    NodeDestroyRecursive(root2);

    NodeDestroy(root_child0);
    NodeDestroy(root_child1);

    ConfigDestroy(config);

    REQUIRE(0 == TestUtil::StopCountingNodes());
  }

  TEST_CASE("LayoutEngine::mixed_shared_and_owned_children", "[LayoutEngine]")
  {
    // Don't try this at home!

    NodeRef root0 = NodeCreate();
    NodeRef root1 = NodeCreate();

    NodeRef root0_child0 = NodeCreate();
    NodeRef root0_child0_0 = NodeCreate();
    NodeInsertChild(root0, root0_child0, 0);
    NodeInsertChild(root0_child0, root0_child0_0, 0);

    NodeRef root1_child0 = NodeCreate();
    NodeRef root1_child2 = NodeCreate();
    NodeInsertChild(root1, root1_child0, 0);
    NodeInsertChild(root1, root1_child2, 1);

    auto children = static_cast<NodeRef>(root1)->GetChildren();
    children.insert(children.begin() + 1, static_cast<NodeRef>(root0_child0));
    static_cast<NodeRef>(root1)->SetChildren(children);

    auto secondChild = NodeGetChild(root1, 1);
    REQUIRE(secondChild == NodeGetChild(root0, 0));
    REQUIRE(NodeGetChild(secondChild, 0) == NodeGetChild(root0_child0, 0));

    NodeCalculateLayout(root1, Undefined, Undefined, Direction::LTR);
    secondChild = NodeGetChild(root1, 1);
    REQUIRE(secondChild != NodeGetChild(root0, 0));
    REQUIRE(NodeGetOwner(secondChild) == root1);
    REQUIRE(NodeGetChild(secondChild, 0) != NodeGetChild(root0_child0, 0));
    REQUIRE(NodeGetOwner(NodeGetChild(secondChild, 0)) == secondChild);
  }
}