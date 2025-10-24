#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI.Layout/Node/Node.hpp"
#include <catch_all.hpp>
#include <cstdio>
namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  TEST_CASE("LayoutEngine::layoutable_children_single_contents_node", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeRef root_child1 = NodeCreate();
    NodeRef root_child2 = NodeCreate();

    NodeRef root_grandchild0 = NodeCreate();
    NodeRef root_grandchild1 = NodeCreate();

    NodeInsertChild(root, root_child0, 0);
    NodeInsertChild(root, root_child1, 1);
    NodeInsertChild(root, root_child2, 2);

    NodeInsertChild(root_child1, root_grandchild0, 0);
    NodeInsertChild(root_child1, root_grandchild1, 1);

    NodeStyleSetDisplay(root_child1, Display::Contents);

    std::vector<NodeRef> order = {
      root_child0,
      root_grandchild0,
      root_grandchild1,
      root_child2,
    };
    auto correctOrderIt = order.begin();

    for (auto node : root->GetLayoutChildren())
    {
      REQUIRE(node == *correctOrderIt);
      correctOrderIt++;
    }

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::layoutable_children_multiple_contents_nodes", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeRef root_child1 = NodeCreate();
    NodeRef root_child2 = NodeCreate();

    NodeRef root_grandchild0 = NodeCreate();
    NodeRef root_grandchild1 = NodeCreate();
    NodeRef root_grandchild2 = NodeCreate();
    NodeRef root_grandchild3 = NodeCreate();
    NodeRef root_grandchild4 = NodeCreate();
    NodeRef root_grandchild5 = NodeCreate();

    NodeInsertChild(root, root_child0, 0);
    NodeInsertChild(root, root_child1, 1);
    NodeInsertChild(root, root_child2, 2);

    NodeInsertChild(root_child0, root_grandchild0, 0);
    NodeInsertChild(root_child0, root_grandchild1, 1);
    NodeInsertChild(root_child1, root_grandchild2, 0);
    NodeInsertChild(root_child1, root_grandchild3, 1);
    NodeInsertChild(root_child2, root_grandchild4, 0);
    NodeInsertChild(root_child2, root_grandchild5, 1);

    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeStyleSetDisplay(root_child1, Display::Contents);
    NodeStyleSetDisplay(root_child2, Display::Contents);

    std::vector<NodeRef> order = {
      root_grandchild0, root_grandchild1, root_grandchild2,
      root_grandchild3, root_grandchild4, root_grandchild5,
    };
    auto correctOrderIt = order.begin();

    for (auto node : root->GetLayoutChildren())
    {
      REQUIRE(node == *correctOrderIt);
      correctOrderIt++;
    }

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::layoutable_children_nested_contents_nodes", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeRef root_child1 = NodeCreate();
    NodeRef root_child2 = NodeCreate();

    NodeRef root_grandchild0 = NodeCreate();
    NodeRef root_grandchild1 = NodeCreate();

    NodeRef root_great_grandchild0 = NodeCreate();
    NodeRef root_great_grandchild1 = NodeCreate();

    NodeInsertChild(root, root_child0, 0);
    NodeInsertChild(root, root_child1, 1);
    NodeInsertChild(root, root_child2, 2);

    NodeInsertChild(root_child1, root_grandchild0, 0);
    NodeInsertChild(root_child1, root_grandchild1, 1);

    NodeInsertChild(root_grandchild1, root_great_grandchild0, 0);
    NodeInsertChild(root_grandchild1, root_great_grandchild1, 1);

    NodeStyleSetDisplay(root_child1, Display::Contents);
    NodeStyleSetDisplay(root_grandchild1, Display::Contents);

    std::vector<NodeRef> order = {
      root_child0, root_grandchild0, root_great_grandchild0, root_great_grandchild1, root_child2,
    };
    auto correctOrderIt = order.begin();

    for (auto node : root->GetLayoutChildren())
    {
      REQUIRE(node == *correctOrderIt);
      correctOrderIt++;
    }

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::layoutable_children_contents_leaf_node", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeRef root_child1 = NodeCreate();
    NodeRef root_child2 = NodeCreate();

    NodeInsertChild(root, root_child0, 0);
    NodeInsertChild(root, root_child1, 1);
    NodeInsertChild(root, root_child2, 2);

    NodeStyleSetDisplay(root_child1, Display::Contents);

    std::vector<NodeRef> order = {
      root_child0,
      root_child2,
    };
    auto correctOrderIt = order.begin();

    for (auto node : root->GetLayoutChildren())
    {
      REQUIRE(node == *correctOrderIt);
      correctOrderIt++;
    }

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::layoutable_children_contents_root_node", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeRef root_child1 = NodeCreate();
    NodeRef root_child2 = NodeCreate();

    NodeInsertChild(root, root_child0, 0);
    NodeInsertChild(root, root_child1, 1);
    NodeInsertChild(root, root_child2, 2);

    NodeStyleSetDisplay(root, Display::Contents);

    std::vector<NodeRef> order = {
      root_child0,
      root_child1,
      root_child2,
    };
    auto correctOrderIt = order.begin();

    for (auto node : root->GetLayoutChildren())
    {
      REQUIRE(node == *correctOrderIt);
      correctOrderIt++;
    }

    NodeDestroyRecursive(root);
  }
}