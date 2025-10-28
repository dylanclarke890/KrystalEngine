#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::justify_content_row_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(92 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(82 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(72 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(72 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(82 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(92 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(56 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(56 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(36 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_space_between", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(92 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(92 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(12 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(46 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(10 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(12 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(10 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(72 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(82 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(92 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(72 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(82 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(92 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(36 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(56 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(36 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(56 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_space_between", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(92 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(92 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(12 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(80 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(12 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(80 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_min_width_and_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Left, 100);
    NodeStyleSetMinWidth(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_max_width_and_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Left, 100);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetMaxWidth(root, 80);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(100 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_min_height_and_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Top, 100);
    NodeStyleSetMinHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_max_height_and_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMargin(root, Edge::Top, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetMaxHeight(root, 80);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(100 == NodeLayoutGetTop(root));
    REQUIRE(20 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_column_space_evenly", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(74 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(102 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(46 == NodeLayoutGetTop(root_child1));
    REQUIRE(102 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(74 == NodeLayoutGetTop(root_child2));
    REQUIRE(102 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_row_space_evenly", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

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
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(26 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(51 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(77 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(0 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(77 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(51 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(26 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(0 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_min_width_with_padding_child_width_greater_than_parent",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 1'000);
    NodeStyleSetHeight(root, 1'584);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0, Align::Stretch);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0_child0, Align::Stretch);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 100);
    NodeStyleSetMinWidth(root_child0_child0, 400);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child0, Align::Stretch);
    NodeStyleSetWidth(root_child0_child0_child0, 300);
    NodeStyleSetHeight(root_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'000 == NodeLayoutGetWidth(root));
    REQUIRE(1'584 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'000 == NodeLayoutGetWidth(root));
    REQUIRE(1'584 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(300 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_min_width_with_padding_child_width_lower_than_parent",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 1'080);
    NodeStyleSetHeight(root, 1'584);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0, Align::Stretch);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0_child0, Align::Stretch);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 100);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 100);
    NodeStyleSetMinWidth(root_child0_child0, 400);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child0, Align::Stretch);
    NodeStyleSetWidth(root_child0_child0_child0, 199);
    NodeStyleSetHeight(root_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'080 == NodeLayoutGetWidth(root));
    REQUIRE(1'584 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(101 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(199 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'080 == NodeLayoutGetWidth(root));
    REQUIRE(1'584 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(680 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(101 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(199 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_space_between_indefinite_container_dim_with_free_space",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::SpaceBetween);
    NodeStyleSetMinWidth(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_flex_start_row_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_flex_end_row_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(62 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(-9 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(31 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(71 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(71 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(31 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-9 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_space_between", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_space_evenly", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_reverse_space_around", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetJustifyContent(root, Justify::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(62 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_reverse_space_evenly", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::RowReverse);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(62 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::justify_content_overflow_row_space_evenly_auto_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 102);
    NodeStyleSetHeight(root, 102);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMarginAuto(root_child0, Edge::Right);
    NodeStyleSetWidth(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 40);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 40);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(102 == NodeLayoutGetWidth(root));
    REQUIRE(102 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(102 == NodeLayoutGetHeight(root_child0));

    REQUIRE(22 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(102 == NodeLayoutGetHeight(root_child1));

    REQUIRE(-18 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(40 == NodeLayoutGetWidth(root_child2));
    REQUIRE(102 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
