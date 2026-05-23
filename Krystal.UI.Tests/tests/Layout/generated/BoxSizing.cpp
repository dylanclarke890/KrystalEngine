#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::box_sizing_content_box_simple", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(130 == NodeLayoutGetWidth(root));
    REQUIRE(130 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(130 == NodeLayoutGetWidth(root));
    REQUIRE(130 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_simple", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 4);
    NodeStyleSetPadding(root_child0, Edge::Top, 4);
    NodeStyleSetPadding(root_child0, Edge::Right, 4);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0, Edge::Left, 16);
    NodeStyleSetBorder(root_child0, Edge::Top, 16);
    NodeStyleSetBorder(root_child0, Edge::Right, 16);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 16);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeStyleSetHeightPercent(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 4);
    NodeStyleSetPadding(root_child0, Edge::Top, 4);
    NodeStyleSetPadding(root_child0, Edge::Right, 4);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0, Edge::Left, 16);
    NodeStyleSetBorder(root_child0, Edge::Top, 16);
    NodeStyleSetBorder(root_child0, Edge::Right, 16);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 16);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeStyleSetHeightPercent(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_absolute", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0, Edge::Left, 12);
    NodeStyleSetPadding(root_child0, Edge::Top, 12);
    NodeStyleSetPadding(root_child0, Edge::Right, 12);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 12);
    NodeStyleSetBorder(root_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0, Edge::Top, 8);
    NodeStyleSetBorder(root_child0, Edge::Right, 8);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetHeightPercent(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_absolute", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPadding(root_child0, Edge::Left, 12);
    NodeStyleSetPadding(root_child0, Edge::Top, 12);
    NodeStyleSetPadding(root_child0, Edge::Right, 12);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 12);
    NodeStyleSetBorder(root_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0, Edge::Top, 8);
    NodeStyleSetBorder(root_child0, Edge::Right, 8);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetHeightPercent(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_comtaining_block", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 12);
    NodeStyleSetPadding(root, Edge::Top, 12);
    NodeStyleSetPadding(root, Edge::Right, 12);
    NodeStyleSetPadding(root, Edge::Bottom, 12);
    NodeStyleSetBorder(root, Edge::Left, 8);
    NodeStyleSetBorder(root, Edge::Top, 8);
    NodeStyleSetBorder(root, Edge::Right, 8);
    NodeStyleSetBorder(root, Edge::Bottom, 8);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeightPercent(root_child0_child0, 25);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(31 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(31 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_comtaining_block", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 12);
    NodeStyleSetPadding(root, Edge::Top, 12);
    NodeStyleSetPadding(root, Edge::Right, 12);
    NodeStyleSetPadding(root, Edge::Bottom, 12);
    NodeStyleSetBorder(root, Edge::Left, 8);
    NodeStyleSetBorder(root, Edge::Top, 8);
    NodeStyleSetBorder(root, Edge::Right, 8);
    NodeStyleSetBorder(root, Edge::Bottom, 8);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeightPercent(root_child0_child0, 25);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_padding_only", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(110 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(110 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_padding_only_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 150);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child0, Edge::Left, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Top, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Right, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 75);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(70 == NodeLayoutGetWidth(root_child0));
    REQUIRE(95 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(70 == NodeLayoutGetWidth(root_child0));
    REQUIRE(95 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_padding_only", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_padding_only_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 150);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child0, Edge::Left, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Top, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Right, 10);
    NodeStyleSetPaddingPercent(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 75);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_border_only", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(120 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(120 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_border_only_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_border_only", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_border_only_percent", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_no_padding_no_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_no_padding_no_border", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_children", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 25);
    NodeStyleSetHeight(root_child3, 25);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(130 == NodeLayoutGetWidth(root));
    REQUIRE(130 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    REQUIRE(15 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(15 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(130 == NodeLayoutGetWidth(root));
    REQUIRE(130 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(90 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_children", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 5);
    NodeStyleSetPadding(root, Edge::Top, 5);
    NodeStyleSetPadding(root, Edge::Right, 5);
    NodeStyleSetPadding(root, Edge::Bottom, 5);
    NodeStyleSetBorder(root, Edge::Left, 10);
    NodeStyleSetBorder(root, Edge::Top, 10);
    NodeStyleSetBorder(root, Edge::Right, 10);
    NodeStyleSetBorder(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 25);
    NodeStyleSetHeight(root_child3, 25);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    REQUIRE(15 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(15 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(60 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_siblings", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPadding(root_child1, Edge::Left, 10);
    NodeStyleSetPadding(root_child1, Edge::Top, 10);
    NodeStyleSetPadding(root_child1, Edge::Right, 10);
    NodeStyleSetPadding(root_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child1, Edge::Left, 10);
    NodeStyleSetBorder(root_child1, Edge::Top, 10);
    NodeStyleSetBorder(root_child1, Edge::Right, 10);
    NodeStyleSetBorder(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeStyleSetBoxSizing(root_child1, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 25);
    NodeStyleSetHeight(root_child3, 25);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(65 == NodeLayoutGetWidth(root_child1));
    REQUIRE(65 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(90 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(115 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(35 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(65 == NodeLayoutGetWidth(root_child1));
    REQUIRE(65 == NodeLayoutGetHeight(root_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child2));
    REQUIRE(90 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(75 == NodeLayoutGetLeft(root_child3));
    REQUIRE(115 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_siblings", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 25);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPadding(root_child1, Edge::Left, 10);
    NodeStyleSetPadding(root_child1, Edge::Top, 10);
    NodeStyleSetPadding(root_child1, Edge::Right, 10);
    NodeStyleSetPadding(root_child1, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child1, Edge::Left, 10);
    NodeStyleSetBorder(root_child1, Edge::Top, 10);
    NodeStyleSetBorder(root_child1, Edge::Right, 10);
    NodeStyleSetBorder(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 25);
    NodeStyleSetHeight(root_child3, 25);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child2));
    REQUIRE(65 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    REQUIRE(75 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(25 == NodeLayoutGetWidth(root_child3));
    REQUIRE(25 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_max_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetMaxWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(65 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(65 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_max_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetMaxWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetMaxHeight(root_child0, 50);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetMaxHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_min_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetMinWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(65 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(65 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(65 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_min_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetMinWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_min_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetMinHeight(root_child0, 50);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_min_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 15);
    NodeStyleSetBorder(root_child0, Edge::Top, 15);
    NodeStyleSetBorder(root_child0, Edge::Right, 15);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 15);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetMinHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 25);
    NodeStyleSetHeight(root_child1, 25);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1));
    REQUIRE(25 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_no_height_no_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 2);
    NodeStyleSetPadding(root_child0, Edge::Right, 2);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 2);
    NodeStyleSetBorder(root_child0, Edge::Left, 7);
    NodeStyleSetBorder(root_child0, Edge::Top, 7);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 7);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(18 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(18 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_no_height_no_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 2);
    NodeStyleSetPadding(root_child0, Edge::Right, 2);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 2);
    NodeStyleSetBorder(root_child0, Edge::Left, 7);
    NodeStyleSetBorder(root_child0, Edge::Top, 7);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 7);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(18 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(18 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_nested", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 15);
    NodeStyleSetPadding(root, Edge::Top, 15);
    NodeStyleSetPadding(root, Edge::Right, 15);
    NodeStyleSetPadding(root, Edge::Bottom, 15);
    NodeStyleSetBorder(root, Edge::Left, 3);
    NodeStyleSetBorder(root, Edge::Top, 3);
    NodeStyleSetBorder(root, Edge::Right, 3);
    NodeStyleSetBorder(root, Edge::Bottom, 3);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 2);
    NodeStyleSetPadding(root_child0, Edge::Right, 2);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 2);
    NodeStyleSetBorder(root_child0, Edge::Left, 7);
    NodeStyleSetBorder(root_child0, Edge::Top, 7);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 7);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 2);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 5);
    NodeStyleSetBoxSizing(root_child0_child0, BoxSizing::ContentBox);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(136 == NodeLayoutGetWidth(root));
    REQUIRE(136 == NodeLayoutGetHeight(root));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(38 == NodeLayoutGetWidth(root_child0));
    REQUIRE(38 == NodeLayoutGetHeight(root_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(16 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(11 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(136 == NodeLayoutGetWidth(root));
    REQUIRE(136 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(38 == NodeLayoutGetWidth(root_child0));
    REQUIRE(38 == NodeLayoutGetHeight(root_child0));

    REQUIRE(13 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(16 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(11 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_nested", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 15);
    NodeStyleSetPadding(root, Edge::Top, 15);
    NodeStyleSetPadding(root, Edge::Right, 15);
    NodeStyleSetPadding(root, Edge::Bottom, 15);
    NodeStyleSetBorder(root, Edge::Left, 3);
    NodeStyleSetBorder(root, Edge::Top, 3);
    NodeStyleSetBorder(root, Edge::Right, 3);
    NodeStyleSetBorder(root, Edge::Bottom, 3);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 2);
    NodeStyleSetPadding(root_child0, Edge::Right, 2);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 2);
    NodeStyleSetBorder(root_child0, Edge::Left, 7);
    NodeStyleSetBorder(root_child0, Edge::Top, 7);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 7);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 2);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 5);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(18 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(6 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(62 == NodeLayoutGetLeft(root_child0));
    REQUIRE(18 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(1 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(6 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_nested_alternating", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 3);
    NodeStyleSetPadding(root, Edge::Top, 3);
    NodeStyleSetPadding(root, Edge::Right, 3);
    NodeStyleSetPadding(root, Edge::Bottom, 3);
    NodeStyleSetBorder(root, Edge::Left, 2);
    NodeStyleSetBorder(root, Edge::Top, 2);
    NodeStyleSetBorder(root, Edge::Right, 2);
    NodeStyleSetBorder(root, Edge::Bottom, 2);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 8);
    NodeStyleSetPadding(root_child0, Edge::Top, 8);
    NodeStyleSetPadding(root_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 8);
    NodeStyleSetBorder(root_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0, Edge::Top, 2);
    NodeStyleSetBorder(root_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 2);
    NodeStyleSetWidth(root_child0, 40);
    NodeStyleSetHeight(root_child0, 40);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 3);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 6);
    NodeStyleSetWidth(root_child0_child0, 20);
    NodeStyleSetHeight(root_child0_child0, 25);
    NodeStyleSetBoxSizing(root_child0_child0, BoxSizing::ContentBox);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0_child0, 5);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(110 == NodeLayoutGetHeight(root));

    REQUIRE(5 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(38 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(43 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(5 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(110 == NodeLayoutGetWidth(root));
    REQUIRE(110 == NodeLayoutGetHeight(root));

    REQUIRE(65 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(-8 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(38 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(43 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(19 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(5 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_nested_alternating", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 3);
    NodeStyleSetPadding(root, Edge::Top, 3);
    NodeStyleSetPadding(root, Edge::Right, 3);
    NodeStyleSetPadding(root, Edge::Bottom, 3);
    NodeStyleSetBorder(root, Edge::Left, 2);
    NodeStyleSetBorder(root, Edge::Top, 2);
    NodeStyleSetBorder(root, Edge::Right, 2);
    NodeStyleSetBorder(root, Edge::Bottom, 2);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 8);
    NodeStyleSetPadding(root_child0, Edge::Top, 8);
    NodeStyleSetPadding(root_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 8);
    NodeStyleSetBorder(root_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0, Edge::Top, 2);
    NodeStyleSetBorder(root_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 2);
    NodeStyleSetWidth(root_child0, 40);
    NodeStyleSetHeight(root_child0, 40);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 3);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 3);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 6);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 6);
    NodeStyleSetWidth(root_child0_child0, 20);
    NodeStyleSetHeight(root_child0_child0, 25);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 1);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0_child0, 5);
    NodeStyleSetBoxSizing(root_child0_child0_child0, BoxSizing::ContentBox);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(5 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(60 == NodeLayoutGetHeight(root_child0));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(9 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(14 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(35 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(60 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-3 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(14 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(9 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_flex_basis_row", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 10);
    NodeStyleSetBorder(root_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0, Edge::Right, 10);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 10);
    NodeStyleSetHeight(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(55 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(55 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_flex_basis_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 10);
    NodeStyleSetBorder(root_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0, Edge::Right, 10);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 10);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_flex_basis_column", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 10);
    NodeStyleSetBorder(root_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0, Edge::Right, 10);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 10);
    NodeStyleSetHeight(root_child0, 25);
    NodeStyleSetBoxSizing(root_child0, BoxSizing::ContentBox);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_flex_basis_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetPadding(root_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0, Edge::Top, 5);
    NodeStyleSetPadding(root_child0, Edge::Right, 5);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 5);
    NodeStyleSetBorder(root_child0, Edge::Left, 10);
    NodeStyleSetBorder(root_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0, Edge::Right, 10);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 10);
    NodeStyleSetHeight(root_child0, 25);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_padding_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Start, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_padding_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Start, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_padding_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::End, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_padding_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::End, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_border_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Start, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_border_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Start, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_content_box_border_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::End, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(105 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::box_sizing_border_box_border_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::End, 5);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
