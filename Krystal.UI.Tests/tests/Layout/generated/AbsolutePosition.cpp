#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::absolute_layout_width_height_start_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Start, 10);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_width_height_left_auto_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPositionAuto(root_child0, Edge::Left);
    NodeStyleSetPosition(root_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_width_height_left_right_auto", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 10);
    NodeStyleSetPositionAuto(root_child0, Edge::Right);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_width_height_left_auto_right_auto", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPositionAuto(root_child0, Edge::Left);
    NodeStyleSetPositionAuto(root_child0, Edge::Right);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_width_height_end_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::End, 10);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_start_top_end_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Start, 10);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetPosition(root_child0, Edge::End, 10);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_width_height_start_top_end_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Start, 10);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetPosition(root_child0, Edge::End, 10);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::do_not_clamp_height_of_absolute_node_to_height_of_its_overflow_hidden_parent",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetOverflow(root, Overflow::Hidden);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Start, 0);
    NodeStyleSetPosition(root_child0, Edge::Top, 0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_within_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Left, 10);
    NodeStyleSetMargin(root, Edge::Top, 10);
    NodeStyleSetMargin(root, Edge::Right, 10);
    NodeStyleSetMargin(root, Edge::Bottom, 10);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 0);
    NodeStyleSetPosition(root_child0, Edge::Top, 0);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPositionType(root_child1, PositionType::Absolute);
    NodeStyleSetPosition(root_child1, Edge::Right, 0);
    NodeStyleSetPosition(root_child1, Edge::Bottom, 0);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetPositionType(root_child2, PositionType::Absolute);
    NodeStyleSetPosition(root_child2, Edge::Left, 0);
    NodeStyleSetPosition(root_child2, Edge::Top, 0);
    NodeStyleSetMargin(root_child2, Edge::Left, 10);
    NodeStyleSetMargin(root_child2, Edge::Top, 10);
    NodeStyleSetMargin(root_child2, Edge::Right, 10);
    NodeStyleSetMargin(root_child2, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetPositionType(root_child3, PositionType::Absolute);
    NodeStyleSetPosition(root_child3, Edge::Right, 0);
    NodeStyleSetPosition(root_child3, Edge::Bottom, 0);
    NodeStyleSetMargin(root_child3, Edge::Left, 10);
    NodeStyleSetMargin(root_child3, Edge::Top, 10);
    NodeStyleSetMargin(root_child3, Edge::Right, 10);
    NodeStyleSetMargin(root_child3, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(30 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(30 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_justify_content_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_center_on_child_only", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::Center);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_center_and_top_position",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_center_and_bottom_position",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_center_and_left_position",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 5);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(5 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(5 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_align_items_and_justify_content_center_and_right_position",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexGrow(root, 1);
    NodeStyleSetWidth(root, 110);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Right, 5);
    NodeStyleSetWidth(root_child0, 60);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(45 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(45 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::position_root_with_rtl_should_position_withoutdirection", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPosition(root, Edge::Left, 72);
    NodeStyleSetWidth(root, 52);
    NodeStyleSetHeight(root, 52);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(72 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(52 == NodeLayoutGetWidth(root));
    REQUIRE(52 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(72 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(52 == NodeLayoutGetWidth(root));
    REQUIRE(52 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_percentage_bottom_based_on_parent_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPositionType(root_child1, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child1, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child1, 10);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetPositionType(root_child2, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child2, Edge::Top, 10);
    NodeStyleSetPositionPercent(root_child2, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(160 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(160 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_in_wrap_reverse_column_container", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_in_wrap_reverse_row_container", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_in_wrap_reverse_column_container_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexEnd);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_in_wrap_reverse_row_container_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexEnd);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::percent_absolute_position_infinite_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 300);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPositionType(root_child1, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child1, Edge::Left, 20);
    NodeStyleSetPositionPercent(root_child1, Edge::Top, 20);
    NodeStyleSetWidthPercent(root_child1, 20);
    NodeStyleSetHeightPercent(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(0 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(60 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(0 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(60 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_percentage_height_based_on_padded_parent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeightPercent(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(45 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(45 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_percentage_height_based_on_padded_parent_and_align_items_center",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPadding(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::Bottom, 20);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeightPercent(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_padding_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 100);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_padding_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Right, 100);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_padding_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Top, 100);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_padding_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Bottom, 100);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 50);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 50);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 50);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(220 == NodeLayoutGetWidth(root));
    REQUIRE(220 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(220 == NodeLayoutGetWidth(root));
    REQUIRE(220 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(220 == NodeLayoutGetWidth(root));
    REQUIRE(220 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(220 == NodeLayoutGetWidth(root));
    REQUIRE(220 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(140 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::absolute_layout_column_reverse_margin_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 5);
    NodeStyleSetPosition(root_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0, Edge::Left, 3);
    NodeStyleSetMargin(root_child0, Edge::Right, 4);
    NodeStyleSetBorder(root_child0, Edge::Left, 1);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(8 == NodeLayoutGetLeft(root_child0));
    REQUIRE(150 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(143 == NodeLayoutGetLeft(root_child0));
    REQUIRE(150 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
