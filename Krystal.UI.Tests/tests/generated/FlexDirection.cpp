#include "../utils/TestUtil.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::flex_direction_column_no_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_no_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(30 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(30 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(70 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(70 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_margin_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Left, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_margin_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Start, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_margin_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Right, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_margin_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::End, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_margin_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Top, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_margin_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Bottom, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_padding_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_padding_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Start, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_padding_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Right, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(-20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_padding_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::End, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(-20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_padding_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Top, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_padding_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Bottom, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_border_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_border_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Start, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_border_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Right, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(-20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_border_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::End, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(-20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_border_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Top, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_border_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::ColumnReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Bottom, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_pos_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetPosition(root_child0, Edge::Left, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_pos_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetPosition(root_child0, Edge::Start, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_pos_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetPosition(root_child0, Edge::Right, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_pos_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetPosition(root_child0, Edge::End, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_pos_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPosition(root_child0, Edge::Top, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_column_reverse_pos_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 100);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(-100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(-100 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_pos_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::Left, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_pos_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_pos_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_pos_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_pos_start", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::Start, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_pos_end", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0, Edge::End, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_margin_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_margin_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_margin_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_margin_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_marign_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::Start, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_margin_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetMargin(root_child0_child0, Edge::End, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_border_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_border_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_border_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_border_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_border_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_border_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_padding_left", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_padding_right", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_padding_top", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_col_reverse_inner_padding_bottom", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::ColumnReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_padding_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::Start, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_row_reverse_inner_padding_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0_child0, Edge::End, 10);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 10);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_direction_alternating_with_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetPositionPercent(root_child0, Edge::Left, 10);
    NodeStyleSetPositionPercent(root_child0, Edge::Top, 10);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeStyleSetHeightPercent(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(120 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
