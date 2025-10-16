#include "../utils/TestUtil.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::static_position_insets_have_no_effect_left_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeStyleSetPosition(root_child0, Edge::Left, 50);
    NodeStyleSetPosition(root_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_insets_have_no_effect_right_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeStyleSetPosition(root_child0, Edge::Right, 50);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_insets_relative_to_positioned_ancestor",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_insets_relative_to_positioned_ancestor_row_reverse",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_reverse_static_position_absolute_child_insets_relative_to_positioned_"
            "ancestor_row_reverse",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_insets_relative_to_positioned_ancestor_row",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE(
    "LayoutEngine::column_reverse_static_position_absolute_child_insets_relative_to_positioned_ancestor_row",
    "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE(
    "LayoutEngine::static_position_absolute_child_insets_relative_to_positioned_ancestor_column_reverse",
    "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_reverse_static_position_absolute_child_insets_relative_to_positioned_"
            "ancestor_column_reverse",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_insets_relative_to_positioned_ancestor_deep",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0_child0_child0_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0_child0_child0, root_child0_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPosition(root_child0_child0_child0_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0_child0_child0, root_child0_child0_child0_child0_child0_child0,
                    0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0));

    REQUIRE(-350 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_width_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_width_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_width_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_height_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_height_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_height_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_left_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_left_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_left_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_right_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_right_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_right_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_top_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_top_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_top_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_bottom_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_bottom_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_bottom_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_margin_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_margin_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_margin_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetMarginPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_padding_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_padding_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_padding_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Left, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Top, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Right, 50);
    NodeStyleSetPaddingPercent(root_child0_child0_child0, Edge::Bottom, 50);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_border_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_border_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_border_percentage", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_containing_block_padding_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_containing_block_padding_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_containing_block_padding_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Static);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_containing_block_content_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_relative_child_containing_block_content_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_child_containing_block_content_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidthPercent(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_containing_block_padding_and_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 9);
    NodeStyleSetPadding(root_child0, Edge::Top, 8);
    NodeStyleSetPadding(root_child0, Edge::Right, 1);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0, Edge::Top, 5);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 4);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 61);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(11 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(13 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(239 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(292 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(13 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-60 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(239 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(1 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-2 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_no_position_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-15 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_zero_for_inset_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 0);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-1 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-265 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_start_inset_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Start, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(11 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-2 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_end_inset_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::End, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(270 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-261 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_row_reverse_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 12);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-128 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(133 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(23 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(133 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(23 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_column_reverse_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-82 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-15 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(-82 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_justify_flex_start_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(111 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_justify_flex_start_position_set_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 30);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(106 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(106 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_no_definite_size_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 23);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(133 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(133 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_both_insets_set_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPositionPercent(root_child0_child0_child0, Edge::Left, 23);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 13);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(69 == NodeLayoutGetWidth(root));
    REQUIRE(79 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(56 == NodeLayoutGetWidth(root_child0));
    REQUIRE(73 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(22 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-3 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_justify_center_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetJustifyContent(root_child0_child0, Justify::Center);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(85 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(111 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(85 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_justify_flex_end_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetJustifyContent(root_child0_child0, Justify::FlexEnd);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(111 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_align_flex_start_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignItems(root_child0_child0, Align::FlexStart);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(111 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_align_center_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignItems(root_child0_child0, Align::Center);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(65 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(39 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(65 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(39 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_align_flex_end_amalgamation", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignItems(root_child0_child0, Align::FlexEnd);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 21);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child1, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child1_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child1_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child1_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child2, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child2, 2);

    NodeRef root_child0_child0_child2_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child2_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child2_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child2_child0, Edge::Bottom, 9);
    NodeStyleSetWidth(root_child0_child0_child2_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child2_child0, 50);
    NodeInsertChild(root_child0_child0_child2, root_child0_child0_child2_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(111 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(131 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(215 == NodeLayoutGetWidth(root));
    REQUIRE(301 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(202 == NodeLayoutGetWidth(root_child0));
    REQUIRE(295 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(166 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(244 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(-77 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0_child0_child2));
    REQUIRE(140 == NodeLayoutGetTop(root_child0_child0_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0_child2));
    REQUIRE(92 == NodeLayoutGetHeight(root_child0_child0_child2));

    REQUIRE(-97 == NodeLayoutGetLeft(root_child0_child0_child2_child0));
    REQUIRE(16 == NodeLayoutGetTop(root_child0_child0_child2_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child2_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child2_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_static_root", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Static);
    NodeStyleSetPadding(root, Edge::Left, 6);
    NodeStyleSetPadding(root, Edge::Top, 1);
    NodeStyleSetPadding(root, Edge::Right, 11);
    NodeStyleSetPadding(root, Edge::Bottom, 4);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0, Edge::Left, 12);
    NodeStyleSetMargin(root_child0, Edge::Top, 11);
    NodeStyleSetMargin(root_child0, Edge::Right, 15);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 3);
    NodeStyleSetPadding(root_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0, Edge::Left, 4);
    NodeStyleSetBorder(root_child0, Edge::Top, 3);
    NodeStyleSetBorder(root_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 1);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeStyleSetHeightPercent(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(12 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(24 == NodeLayoutGetLeft(root_child0));
    REQUIRE(12 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::static_position_absolute_child_multiple", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 100);
    NodeStyleSetPadding(root_child0, Edge::Right, 100);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 400);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0_child0, 50);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child1, PositionType::Static);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child1_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child1_child0, 50);
    NodeStyleSetHeight(root_child0_child1_child0, 50);
    NodeInsertChild(root_child0_child1, root_child0_child1_child0, 0);

    NodeRef root_child0_child1_child1 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child1_child1, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child1_child1, 50);
    NodeStyleSetHeight(root_child0_child1_child1, 50);
    NodeInsertChild(root_child0_child1, root_child0_child1_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child2, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child1_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1_child0));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child1_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1_child1));

    REQUIRE(275 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
