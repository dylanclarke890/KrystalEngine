#include "Krystal.UI/LayoutEngine.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  static List<NodeRef> GetChildren(NodeRef const node)
  {
    const auto count = NodeGetChildCount(node);
    List<NodeRef> children;
    children.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
      children.push_back(NodeGetChild(node, i));
    }
    return children;
  }

  TEST_CASE("LayoutEngine::set_children_adds_children_to_parent", "[LayoutEngine]")
  {
    NodeRef const root = NodeCreate();
    NodeRef const root_child0 = NodeCreate();
    NodeRef const root_child1 = NodeCreate();

    NodeRef children[] = {root_child0, root_child1};
    NodeSetChildren(root, children, 2);

    const List<NodeRef> expectedChildren = {root_child0, root_child1};
    REQUIRE(GetChildren(root) == expectedChildren);

    const List<NodeRef> owners = {NodeGetOwner(root_child0), NodeGetOwner(root_child1)};
    const List<NodeRef> expectedOwners = {root, root};
    REQUIRE(owners == expectedOwners);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::set_children_to_empty_removes_old_children", "[LayoutEngine]")
  {
    NodeRef const root = NodeCreate();
    NodeRef const root_child0 = NodeCreate();
    NodeRef const root_child1 = NodeCreate();

    NodeRef children[] = {root_child0, root_child1};
    NodeSetChildren(root, children, 2);
    NodeSetChildren(root, nullptr, 0);

    const List<NodeRef> expectedChildren = {};
    REQUIRE(GetChildren(root) == expectedChildren);

    const List<NodeRef> owners = {NodeGetOwner(root_child0), NodeGetOwner(root_child1)};
    const List<NodeRef> expectedOwners = {nullptr, nullptr};
    REQUIRE(owners == expectedOwners);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::set_children_replaces_non_common_children", "[LayoutEngine]")
  {
    NodeRef const root = NodeCreate();
    NodeRef const root_child0 = NodeCreate();
    NodeRef const root_child1 = NodeCreate();

    NodeRef children1[] = {root_child0, root_child1};
    NodeSetChildren(root, children1, 2);

    NodeRef const root_child2 = NodeCreate();
    NodeRef const root_child3 = NodeCreate();

    NodeRef children2[] = {root_child2, root_child3};
    NodeSetChildren(root, children2, 2);

    const List<NodeRef> expectedChildren = {root_child2, root_child3};
    REQUIRE(GetChildren(root) == expectedChildren);

    const List<NodeRef> owners = {NodeGetOwner(root_child0), NodeGetOwner(root_child1)};
    const List<NodeRef> expectedOwners = {nullptr, nullptr};
    REQUIRE(owners == expectedOwners);

    NodeDestroyRecursive(root);
    NodeDestroy(root_child0);
    NodeDestroy(root_child1);
  }

  TEST_CASE("LayoutEngine::set_children_keeps_and_reorders_common_children", "[LayoutEngine]")
  {
    NodeRef const root = NodeCreate();
    NodeRef const root_child0 = NodeCreate();
    NodeRef const root_child1 = NodeCreate();
    NodeRef const root_child2 = NodeCreate();

    NodeRef children1[] = {root_child0, root_child1, root_child2};
    NodeSetChildren(root, children1, 3);

    NodeRef const root_child3 = NodeCreate();

    NodeRef children2[] = {root_child2, root_child1, root_child3};
    NodeSetChildren(root, children2, 3);

    const List<NodeRef> expectedChildren = {root_child2, root_child1, root_child3};
    REQUIRE(GetChildren(root) == expectedChildren);

    const List<NodeRef> owners = {NodeGetOwner(root_child0), NodeGetOwner(root_child1),
                                  NodeGetOwner(root_child2), NodeGetOwner(root_child3)};
    const List<NodeRef> expectedOwners = {nullptr, root, root, root};
    REQUIRE(owners == expectedOwners);

    NodeDestroyRecursive(root);
    NodeDestroy(root_child0);
  }
}
