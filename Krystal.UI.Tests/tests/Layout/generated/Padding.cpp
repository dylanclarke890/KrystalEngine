#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::padding_no_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::padding_container_match_child", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(30 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(30 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::padding_flex_child", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::padding_stretch_child", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::padding_center_child", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Start, 10);
    NodeStyleSetPadding(root, Edge::End, 20);
    NodeStyleSetPadding(root, Edge::Bottom, 20);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(40 == NodeLayoutGetLeft(root_child0));
    REQUIRE(35 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(35 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::child_with_padding_align_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 20);
    NodeStyleSetPadding(root_child0, Edge::Top, 20);
    NodeStyleSetPadding(root_child0, Edge::Right, 20);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 20);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::physical_and_relative_edge_defined", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 20);
    NodeStyleSetPadding(root, Edge::End, 50);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0, 100);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(130 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
