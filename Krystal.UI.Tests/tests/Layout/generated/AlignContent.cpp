#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::align_content_flex_start_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_without_height_on_children", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_with_flex", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child3, 1);
    NodeStyleSetFlexShrink(root_child3, 1);
    NodeStyleSetFlexBasisPercent(root_child3, 0);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(80 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(120 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(80 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(120 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_end_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_end_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(90 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(90 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_end_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(110 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(110 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(110 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(110 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_end_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::FlexEnd);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(-10 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-50 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(-10 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_end_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::FlexEnd);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-70 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-40 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(-10 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-70 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-40 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(-10 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_center_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_center_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(45 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(45 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(65 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(45 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(45 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(65 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_center_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_center_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::Center);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-25 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-5 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(15 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-25 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-5 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(15 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_center_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::Center);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-35 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-5 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(-35 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(-5 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceBetween);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_wrapped_negative_space_row_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceBetween);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_between_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceBetween);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(15 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(95 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(15 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(95 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceAround);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_wrapped_negative_space_row_reverse", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::RowReverse);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceAround);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceAround);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_nowrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeStyleSetHeight(root_child3, 10);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeStyleSetHeight(root_child4, 10);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(23 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(23 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(88 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(23 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(23 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    REQUIRE(90 == NodeLayoutGetLeft(root_child2));
    REQUIRE(55 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(10 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(55 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(90 == NodeLayoutGetLeft(root_child4));
    REQUIRE(88 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_wrap_singleline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 140);
    NodeStyleSetHeight(root, 120);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 10);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(140 == NodeLayoutGetWidth(root));
    REQUIRE(120 == NodeLayoutGetHeight(root));

    REQUIRE(90 == NodeLayoutGetLeft(root_child0));
    REQUIRE(55 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(55 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_wrapped_negative_space", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceEvenly);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_wrapped_negative_space_gap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetBorder(root, Edge::Left, 60);
    NodeStyleSetBorder(root, Edge::Top, 60);
    NodeStyleSetBorder(root, Edge::Right, 60);
    NodeStyleSetBorder(root, Edge::Bottom, 60);
    NodeStyleSetWidth(root, 320);
    NodeStyleSetHeight(root, 320);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetJustifyContent(root_child0, Justify::Center);
    NodeStyleSetAlignContent(root_child0, Align::SpaceEvenly);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 10);
    NodeStyleSetGap(root_child0, Gutter::Column, 10);
    NodeStyleSetGap(root_child0, Gutter::Row, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child0, 80);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child0_child2, 80);
    NodeStyleSetHeight(root_child0_child2, 20);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(320 == NodeLayoutGetWidth(root));
    REQUIRE(320 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(60 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(160 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(0 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(0 == NodeLayoutGetHeight(root_child3));

    REQUIRE(100 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_children", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_flex", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child3, 1);
    NodeStyleSetFlexShrink(root_child3, 1);
    NodeStyleSetFlexBasisPercent(root_child3, 0);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(0 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(100 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(0 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_flex_no_shrink", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child3, 1);
    NodeStyleSetFlexBasisPercent(root_child3, 0);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(0 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(100 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(0 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1, Edge::Left, 10);
    NodeStyleSetMargin(root_child1, Edge::Top, 10);
    NodeStyleSetMargin(root_child1, Edge::Right, 10);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetMargin(root_child3, Edge::Left, 10);
    NodeStyleSetMargin(root_child3, Edge::Top, 10);
    NodeStyleSetMargin(root_child3, Edge::Right, 10);
    NodeStyleSetMargin(root_child3, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(40 == NodeLayoutGetHeight(root_child2));

    REQUIRE(60 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(40 == NodeLayoutGetHeight(root_child2));

    REQUIRE(40 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(100 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPadding(root_child1, Edge::Left, 10);
    NodeStyleSetPadding(root_child1, Edge::Top, 10);
    NodeStyleSetPadding(root_child1, Edge::Right, 10);
    NodeStyleSetPadding(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetPadding(root_child3, Edge::Left, 10);
    NodeStyleSetPadding(root_child3, Edge::Top, 10);
    NodeStyleSetPadding(root_child3, Edge::Right, 10);
    NodeStyleSetPadding(root_child3, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_single_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_fixed_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 60);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(60 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(80 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(60 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(80 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetMaxHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_row_with_min_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 150);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetMinHeight(root_child1, 80);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(90 == NodeLayoutGetHeight(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(90 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(90 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(90 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(90 == NodeLayoutGetHeight(root_child2));

    REQUIRE(100 == NodeLayoutGetLeft(root_child3));
    REQUIRE(90 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(10 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(90 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(10 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 150);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetHeight(root_child3, 50);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    REQUIRE(50 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(50 == NodeLayoutGetWidth(root_child3));
    REQUIRE(50 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(50 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_is_not_overriding_align_items", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0, Align::Stretch);
    NodeStyleSetAlignItems(root_child0, Align::Center);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0, Align::Stretch);
    NodeStyleSetWidth(root_child0_child0, 10);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);
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
    REQUIRE(45 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

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
    REQUIRE(45 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_with_min_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMinHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(250 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(250 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_with_max_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMaxHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_with_max_cross_axis_and_border_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetPadding(root, Edge::Left, 2);
    NodeStyleSetPadding(root, Edge::Top, 2);
    NodeStyleSetPadding(root, Edge::Right, 2);
    NodeStyleSetPadding(root, Edge::Bottom, 2);
    NodeStyleSetBorder(root, Edge::Left, 5);
    NodeStyleSetBorder(root, Edge::Top, 5);
    NodeStyleSetBorder(root, Edge::Right, 5);
    NodeStyleSetBorder(root, Edge::Bottom, 5);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMaxHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(414 == NodeLayoutGetHeight(root));

    REQUIRE(7 == NodeLayoutGetLeft(root_child0));
    REQUIRE(7 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(7 == NodeLayoutGetLeft(root_child1));
    REQUIRE(207 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(414 == NodeLayoutGetHeight(root));

    REQUIRE(93 == NodeLayoutGetLeft(root_child0));
    REQUIRE(7 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(93 == NodeLayoutGetLeft(root_child1));
    REQUIRE(207 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_with_min_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMinHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(33 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(267 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(33 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(267 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_with_max_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMaxHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_with_max_cross_axis_violated", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMaxHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_with_max_cross_axis_violated_padding_and_border",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetPadding(root, Edge::Left, 2);
    NodeStyleSetPadding(root, Edge::Top, 2);
    NodeStyleSetPadding(root, Edge::Right, 2);
    NodeStyleSetPadding(root, Edge::Bottom, 2);
    NodeStyleSetBorder(root, Edge::Left, 5);
    NodeStyleSetBorder(root, Edge::Top, 5);
    NodeStyleSetBorder(root, Edge::Right, 5);
    NodeStyleSetBorder(root, Edge::Bottom, 5);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetMaxHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 400);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(7 == NodeLayoutGetLeft(root_child0));
    REQUIRE(7 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(7 == NodeLayoutGetLeft(root_child1));
    REQUIRE(207 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(93 == NodeLayoutGetLeft(root_child0));
    REQUIRE(7 == NodeLayoutGetTop(root_child0));
    REQUIRE(400 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(93 == NodeLayoutGetLeft(root_child1));
    REQUIRE(207 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_and_align_items_flex_end_with_flex_wrap",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(88 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(88 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_and_align_items_center_with_flex_wrap",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(63 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(63 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_around_and_align_items_flex_start_with_flex_wrap",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(38 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(38 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_flex_start_stretch_doesnt_influence_line_box_dim", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 20);
    NodeStyleSetPadding(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::Right, 20);
    NodeStyleSetPadding(root, Edge::Bottom, 20);
    NodeStyleSetWidth(root, 400);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child1, Wrap::Wrap);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child0, 30);
    NodeStyleSetHeight(root_child1_child0, 30);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child1, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child1, 30);
    NodeStyleSetHeight(root_child1_child1, 30);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeRef root_child1_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child2, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child2, 30);
    NodeStyleSetHeight(root_child1_child2, 30);
    NodeInsertChild(root_child1, root_child1_child2, 2);

    NodeRef root_child1_child3 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child3, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child3, 30);
    NodeStyleSetHeight(root_child1_child3, 30);
    NodeInsertChild(root_child1, root_child1_child3, 3);

    NodeRef root_child1_child4 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child4, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child4, 30);
    NodeStyleSetHeight(root_child1_child4, 30);
    NodeInsertChild(root_child1, root_child1_child4, 4);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child2, Edge::Left, 20);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(140 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(330 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(260 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_stretch_does_influence_line_box_dim", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 20);
    NodeStyleSetPadding(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::Right, 20);
    NodeStyleSetPadding(root, Edge::Bottom, 20);
    NodeStyleSetWidth(root, 400);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child1, Align::Stretch);
    NodeStyleSetFlexWrap(root_child1, Wrap::Wrap);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child0, 30);
    NodeStyleSetHeight(root_child1_child0, 30);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child1, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child1, 30);
    NodeStyleSetHeight(root_child1_child1, 30);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeRef root_child1_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child2, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child2, 30);
    NodeStyleSetHeight(root_child1_child2, 30);
    NodeInsertChild(root_child1, root_child1_child2, 2);

    NodeRef root_child1_child3 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child3, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child3, 30);
    NodeStyleSetHeight(root_child1_child3, 30);
    NodeInsertChild(root_child1, root_child1_child3, 3);

    NodeRef root_child1_child4 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child4, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child4, 30);
    NodeStyleSetHeight(root_child1_child4, 30);
    NodeInsertChild(root_child1, root_child1_child4, 4);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child2, Edge::Left, 20);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(140 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(330 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(260 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_space_evenly_stretch_does_influence_line_box_dim", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetPadding(root, Edge::Left, 20);
    NodeStyleSetPadding(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::Right, 20);
    NodeStyleSetPadding(root, Edge::Bottom, 20);
    NodeStyleSetWidth(root, 400);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child1, Align::Stretch);
    NodeStyleSetFlexWrap(root_child1, Wrap::Wrap);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child0, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child0, 30);
    NodeStyleSetHeight(root_child1_child0, 30);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child1, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child1, 30);
    NodeStyleSetHeight(root_child1_child1, 30);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeRef root_child1_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child2, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child2, 30);
    NodeStyleSetHeight(root_child1_child2, 30);
    NodeInsertChild(root_child1, root_child1_child2, 2);

    NodeRef root_child1_child3 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child3, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child3, 30);
    NodeStyleSetHeight(root_child1_child3, 30);
    NodeInsertChild(root_child1, root_child1_child3, 3);

    NodeRef root_child1_child4 = NodeCreate(config);
    NodeStyleSetMargin(root_child1_child4, Edge::Right, 20);
    NodeStyleSetWidth(root_child1_child4, 30);
    NodeStyleSetHeight(root_child1_child4, 30);
    NodeInsertChild(root_child1, root_child1_child4, 4);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child2, Edge::Left, 20);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(140 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(330 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(400 == NodeLayoutGetWidth(root));
    REQUIRE(140 == NodeLayoutGetHeight(root));

    REQUIRE(260 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(90 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(170 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child2));

    REQUIRE(120 == NodeLayoutGetLeft(root_child1_child3));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child3));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1_child4));
    REQUIRE(50 == NodeLayoutGetTop(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1_child4));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1_child4));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_and_align_items_flex_end_with_flex_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexStart);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(75 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(250 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(75 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(250 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_and_align_items_flex_start_with_flex_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexEnd);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(175 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(175 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_and_align_items_center_with_flex_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexEnd);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(38 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(213 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_content_stretch_and_align_items_stretch_with_flex_wrap", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignSelf(root_child0, Align::FlexEnd);
    NodeStyleSetWidth(root_child0, 150);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 120);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 120);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(150 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(175 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0));
    REQUIRE(125 == NodeLayoutGetTop(root_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(120 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(175 == NodeLayoutGetTop(root_child2));
    REQUIRE(120 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
