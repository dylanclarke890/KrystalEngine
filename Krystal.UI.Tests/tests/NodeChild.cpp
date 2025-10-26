#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::reset_layout_when_child_removed", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeRemoveChild(root, root_child0);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(Krys::IsUndefined(NodeLayoutGetWidth(root_child0)));
    REQUIRE(Krys::IsUndefined(NodeLayoutGetHeight(root_child0)));

    NodeDestroyRecursive(root);
    NodeDestroyRecursive(root_child0);
  }
}