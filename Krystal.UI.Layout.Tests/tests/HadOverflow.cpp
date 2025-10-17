#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;

  class HadOverflowTests
  {
  protected:
    HadOverflowTests() : config(ConfigCreate())
    {
      root = NodeCreate(config);
      NodeStyleSetWidth(root, 200);
      NodeStyleSetHeight(root, 100);
      NodeStyleSetFlexDirection(root, FlexDirection::Column);
      NodeStyleSetFlexWrap(root, Wrap::NoWrap);
    }

    ~HadOverflowTests()
    {
      NodeDestroyRecursive(root);
      ConfigDestroy(config);
    }

    NodeRef root;
    ConfigRef config;
  };

  TEST_CASE_METHOD(HadOverflowTests, "children_overflow_no_wrap_and_no_flex_children", "[LayoutEngine]")
  {
    NodeRef child0 = NodeCreate(config);
    NodeStyleSetWidth(child0, 80);
    NodeStyleSetHeight(child0, 40);
    NodeStyleSetMargin(child0, Edge::Top, 10);
    NodeStyleSetMargin(child0, Edge::Bottom, 15);
    NodeInsertChild(root, child0, 0);
    NodeRef child1 = NodeCreate(config);
    NodeStyleSetWidth(child1, 80);
    NodeStyleSetHeight(child1, 40);
    NodeStyleSetMargin(child1, Edge::Bottom, 5);
    NodeInsertChild(root, child1, 1);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(NodeLayoutGetHadOverflow(root));
  }

  TEST_CASE_METHOD(HadOverflowTests, "spacing_overflow_no_wrap_and_no_flex_children", "[LayoutEngine]")
  {
    NodeRef child0 = NodeCreate(config);
    NodeStyleSetWidth(child0, 80);
    NodeStyleSetHeight(child0, 40);
    NodeStyleSetMargin(child0, Edge::Top, 10);
    NodeStyleSetMargin(child0, Edge::Bottom, 10);
    NodeInsertChild(root, child0, 0);
    NodeRef child1 = NodeCreate(config);
    NodeStyleSetWidth(child1, 80);
    NodeStyleSetHeight(child1, 40);
    NodeStyleSetMargin(child1, Edge::Bottom, 5);
    NodeInsertChild(root, child1, 1);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(NodeLayoutGetHadOverflow(root));
  }

  TEST_CASE_METHOD(HadOverflowTests, "no_overflow_no_wrap_and_flex_children", "[LayoutEngine]")
  {
    NodeRef child0 = NodeCreate(config);
    NodeStyleSetWidth(child0, 80);
    NodeStyleSetHeight(child0, 40);
    NodeStyleSetMargin(child0, Edge::Top, 10);
    NodeStyleSetMargin(child0, Edge::Bottom, 10);
    NodeInsertChild(root, child0, 0);
    NodeRef child1 = NodeCreate(config);
    NodeStyleSetWidth(child1, 80);
    NodeStyleSetHeight(child1, 40);
    NodeStyleSetMargin(child1, Edge::Bottom, 5);
    NodeStyleSetFlexShrink(child1, 1);
    NodeInsertChild(root, child1, 1);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(!NodeLayoutGetHadOverflow(root));
  }

  TEST_CASE_METHOD(HadOverflowTests, "hadOverflow_gets_reset_if_no_longer_valid", "[LayoutEngine]")
  {
    NodeRef child0 = NodeCreate(config);
    NodeStyleSetWidth(child0, 80);
    NodeStyleSetHeight(child0, 40);
    NodeStyleSetMargin(child0, Edge::Top, 10);
    NodeStyleSetMargin(child0, Edge::Bottom, 10);
    NodeInsertChild(root, child0, 0);
    NodeRef child1 = NodeCreate(config);
    NodeStyleSetWidth(child1, 80);
    NodeStyleSetHeight(child1, 40);
    NodeStyleSetMargin(child1, Edge::Bottom, 5);
    NodeInsertChild(root, child1, 1);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(NodeLayoutGetHadOverflow(root));

    NodeStyleSetFlexShrink(child1, 1);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(!NodeLayoutGetHadOverflow(root));
  }

  TEST_CASE_METHOD(HadOverflowTests, "spacing_overflow_in_nested_nodes", "[LayoutEngine]")
  {
    NodeRef child0 = NodeCreate(config);
    NodeStyleSetWidth(child0, 80);
    NodeStyleSetHeight(child0, 40);
    NodeStyleSetMargin(child0, Edge::Top, 10);
    NodeStyleSetMargin(child0, Edge::Bottom, 10);
    NodeInsertChild(root, child0, 0);
    NodeRef child1 = NodeCreate(config);
    NodeStyleSetWidth(child1, 80);
    NodeStyleSetHeight(child1, 40);
    NodeInsertChild(root, child1, 1);
    NodeRef child1_1 = NodeCreate(config);
    NodeStyleSetWidth(child1_1, 80);
    NodeStyleSetHeight(child1_1, 40);
    NodeStyleSetMargin(child1_1, Edge::Bottom, 5);
    NodeInsertChild(child1, child1_1, 0);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(NodeLayoutGetHadOverflow(root));
  }
}