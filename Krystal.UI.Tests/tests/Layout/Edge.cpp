#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::start_overrides", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Start, 10);
    NodeStyleSetMargin(root_child0, Edge::Left, 20);
    NodeStyleSetMargin(root_child0, Edge::Right, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetRight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);
    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetRight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::end_overrides", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::End, 10);
    NodeStyleSetMargin(root_child0, Edge::Left, 20);
    NodeStyleSetMargin(root_child0, Edge::Right, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetRight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);
    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetRight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::horizontal_overridden", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Horizontal, 10);
    NodeStyleSetMargin(root_child0, Edge::Left, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetRight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::vertical_overridden", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Vertical, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetBottom(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::horizontal_overrides_all", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Horizontal, 10);
    NodeStyleSetMargin(root_child0, Edge::All, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetRight(root_child0));
    REQUIRE(20 == NodeLayoutGetBottom(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::vertical_overrides_all", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Vertical, 10);
    NodeStyleSetMargin(root_child0, Edge::All, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetRight(root_child0));
    REQUIRE(10 == NodeLayoutGetBottom(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::all_overridden", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeStyleSetMargin(root_child0, Edge::All, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetRight(root_child0));
    REQUIRE(10 == NodeLayoutGetBottom(root_child0));

    NodeDestroyRecursive(root);
  }
}