#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::column_gap_flexible", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_inflexible", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_mixed_flexible", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_child_margins", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeStyleSetMargin(root_child0, Edge::Left, 2);
    NodeStyleSetMargin(root_child0, Edge::Right, 2);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetMargin(root_child1, Edge::Left, 10);
    NodeStyleSetMargin(root_child1, Edge::Right, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeStyleSetMargin(root_child2, Edge::Left, 15);
    NodeStyleSetMargin(root_child2, Edge::Right, 15);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(2 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(26 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(2 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(63 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(2 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(76 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(52 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(2 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(15 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(2 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_row_gap_wrapping", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);

    NodeRef root_child6 = NodeCreate(config);
    NodeStyleSetWidth(root_child6, 20);
    NodeStyleSetHeight(root_child6, 20);
    NodeInsertChild(root, root_child6, 6);

    NodeRef root_child7 = NodeCreate(config);
    NodeStyleSetWidth(root_child7, 20);
    NodeStyleSetHeight(root_child7, 20);
    NodeInsertChild(root, root_child7, 7);

    NodeRef root_child8 = NodeCreate(config);
    NodeStyleSetWidth(root_child8, 20);
    NodeStyleSetHeight(root_child8, 20);
    NodeInsertChild(root, root_child8, 8);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(40 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(40 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(40 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    REQUIRE(0 == NodeLayoutGetLeft(root_child6));
    REQUIRE(80 == NodeLayoutGetTop(root_child6));
    REQUIRE(20 == NodeLayoutGetWidth(root_child6));
    REQUIRE(20 == NodeLayoutGetHeight(root_child6));

    REQUIRE(30 == NodeLayoutGetLeft(root_child7));
    REQUIRE(80 == NodeLayoutGetTop(root_child7));
    REQUIRE(20 == NodeLayoutGetWidth(root_child7));
    REQUIRE(20 == NodeLayoutGetHeight(root_child7));

    REQUIRE(60 == NodeLayoutGetLeft(root_child8));
    REQUIRE(80 == NodeLayoutGetTop(root_child8));
    REQUIRE(20 == NodeLayoutGetWidth(root_child8));
    REQUIRE(20 == NodeLayoutGetHeight(root_child8));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(60 == NodeLayoutGetLeft(root_child3));
    REQUIRE(40 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(40 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(0 == NodeLayoutGetLeft(root_child5));
    REQUIRE(40 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    REQUIRE(60 == NodeLayoutGetLeft(root_child6));
    REQUIRE(80 == NodeLayoutGetTop(root_child6));
    REQUIRE(20 == NodeLayoutGetWidth(root_child6));
    REQUIRE(20 == NodeLayoutGetHeight(root_child6));

    REQUIRE(30 == NodeLayoutGetLeft(root_child7));
    REQUIRE(80 == NodeLayoutGetTop(root_child7));
    REQUIRE(20 == NodeLayoutGetWidth(root_child7));
    REQUIRE(20 == NodeLayoutGetHeight(root_child7));

    REQUIRE(0 == NodeLayoutGetLeft(root_child8));
    REQUIRE(80 == NodeLayoutGetTop(root_child8));
    REQUIRE(20 == NodeLayoutGetWidth(root_child8));
    REQUIRE(20 == NodeLayoutGetHeight(root_child8));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_start_index", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 80);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(60 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(70 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(20 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(60 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_space_between", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(80 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(3 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(77 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(77 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(3 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_justify_space_evenly", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceEvenly);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

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

    REQUIRE(5 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(5 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(40 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(40 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(40 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(40 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(40 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(40 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(60 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(60 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(60 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(60 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(60 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(60 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(40 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(80 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(40 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(40 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(80 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_space_between", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceBetween);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(80 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(80 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(80 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(80 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeStyleSetHeight(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeStyleSetHeight(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeStyleSetHeight(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeStyleSetHeight(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(70 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(70 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(70 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(10 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(20 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(70 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(20 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(70 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(20 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(70 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(20 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_wrap_align_stretch", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);
    NodeStyleSetGap(root, Gutter::Column, 5);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetMinWidth(root_child0, 60);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetMinWidth(root_child1, 60);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetMinWidth(root_child2, 60);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child3, 1);
    NodeStyleSetMinWidth(root_child3, 60);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child4, 1);
    NodeStyleSetMinWidth(root_child4, 60);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(71 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(76 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(72 == NodeLayoutGetWidth(root_child1));
    REQUIRE(150 == NodeLayoutGetHeight(root_child1));

    REQUIRE(153 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(71 == NodeLayoutGetWidth(root_child2));
    REQUIRE(150 == NodeLayoutGetHeight(root_child2));

    REQUIRE(229 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(71 == NodeLayoutGetWidth(root_child3));
    REQUIRE(150 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(150 == NodeLayoutGetTop(root_child4));
    REQUIRE(300 == NodeLayoutGetWidth(root_child4));
    REQUIRE(150 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(229 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(71 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(153 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(71 == NodeLayoutGetWidth(root_child1));
    REQUIRE(150 == NodeLayoutGetHeight(root_child1));

    REQUIRE(76 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(72 == NodeLayoutGetWidth(root_child2));
    REQUIRE(150 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(71 == NodeLayoutGetWidth(root_child3));
    REQUIRE(150 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(150 == NodeLayoutGetTop(root_child4));
    REQUIRE(300 == NodeLayoutGetWidth(root_child4));
    REQUIRE(150 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::column_gap_determines_parent_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetGap(root, Gutter::Column, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(20 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(80 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_align_items_stretch", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(90 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(90 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(110 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(90 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(90 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(110 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(90 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(90 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(90 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(110 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(90 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(110 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(90 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(110 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(90 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_align_items_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetGap(root, Gutter::Column, 10);
    NodeStyleSetGap(root, Gutter::Row, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 20);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 20);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 20);
    NodeInsertChild(root, root_child4, 4);

    NodeRef root_child5 = NodeCreate(config);
    NodeStyleSetWidth(root_child5, 20);
    NodeInsertChild(root, root_child5, 5);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(20 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(0 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    REQUIRE(60 == NodeLayoutGetLeft(root_child5));
    REQUIRE(20 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(0 == NodeLayoutGetHeight(root_child5));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(80 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(20 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(20 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(20 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(20 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    REQUIRE(80 == NodeLayoutGetLeft(root_child3));
    REQUIRE(20 == NodeLayoutGetTop(root_child3));
    REQUIRE(20 == NodeLayoutGetWidth(root_child3));
    REQUIRE(0 == NodeLayoutGetHeight(root_child3));

    REQUIRE(50 == NodeLayoutGetLeft(root_child4));
    REQUIRE(20 == NodeLayoutGetTop(root_child4));
    REQUIRE(20 == NodeLayoutGetWidth(root_child4));
    REQUIRE(0 == NodeLayoutGetHeight(root_child4));

    REQUIRE(20 == NodeLayoutGetLeft(root_child5));
    REQUIRE(20 == NodeLayoutGetTop(root_child5));
    REQUIRE(20 == NodeLayoutGetWidth(root_child5));
    REQUIRE(0 == NodeLayoutGetHeight(root_child5));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_column_child_margins", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetGap(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeStyleSetMargin(root_child0, Edge::Top, 2);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 2);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetMargin(root_child1, Edge::Top, 10);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 10);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeStyleSetMargin(root_child2, Edge::Top, 15);
    NodeStyleSetMargin(root_child2, Edge::Bottom, 15);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(2 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(42 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(66 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(42 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(143 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(42 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(2 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(42 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(66 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(42 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(143 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(42 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_row_wrap_child_margins", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetGap(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Top, 2);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 2);
    NodeStyleSetWidth(root_child0, 60);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1, Edge::Top, 10);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 60);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetMargin(root_child2, Edge::Top, 15);
    NodeStyleSetMargin(root_child2, Edge::Bottom, 15);
    NodeStyleSetWidth(root_child2, 60);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(2 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(24 == NodeLayoutGetTop(root_child1));
    REQUIRE(60 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(59 == NodeLayoutGetTop(root_child2));
    REQUIRE(60 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(40 == NodeLayoutGetLeft(root_child0));
    REQUIRE(2 == NodeLayoutGetTop(root_child0));
    REQUIRE(60 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(24 == NodeLayoutGetTop(root_child1));
    REQUIRE(60 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(40 == NodeLayoutGetLeft(root_child2));
    REQUIRE(59 == NodeLayoutGetTop(root_child2));
    REQUIRE(60 == NodeLayoutGetWidth(root_child2));
    REQUIRE(0 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_determines_parent_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetGap(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(20 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 700);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(138 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(138 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(10 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(190 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(62 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(190 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(62 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(190 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_determines_parent_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(130 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(130 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(200 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(200 == NodeLayoutGetLeft(root_child4));
    REQUIRE(200 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_both_content_padding_and_item_padding",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 700);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 10);
    NodeStyleSetPadding(root_child0, Edge::Top, 10);
    NodeStyleSetPadding(root_child0, Edge::Right, 10);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetPadding(root_child1, Edge::Left, 10);
    NodeStyleSetPadding(root_child1, Edge::Top, 10);
    NodeStyleSetPadding(root_child1, Edge::Right, 10);
    NodeStyleSetPadding(root_child1, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetPadding(root_child2, Edge::Left, 10);
    NodeStyleSetPadding(root_child2, Edge::Top, 10);
    NodeStyleSetPadding(root_child2, Edge::Right, 10);
    NodeStyleSetPadding(root_child2, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetPadding(root_child3, Edge::Left, 10);
    NodeStyleSetPadding(root_child3, Edge::Top, 10);
    NodeStyleSetPadding(root_child3, Edge::Right, 10);
    NodeStyleSetPadding(root_child3, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetPadding(root_child4, Edge::Left, 10);
    NodeStyleSetPadding(root_child4, Edge::Top, 10);
    NodeStyleSetPadding(root_child4, Edge::Right, 10);
    NodeStyleSetPadding(root_child4, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(138 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(138 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(10 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(190 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(62 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(190 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(62 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(190 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_both_content_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 700);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(138 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(138 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(10 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(190 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(62 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(190 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(62 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(190 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_content_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetMargin(root, Edge::Left, 10);
    NodeStyleSetMargin(root, Edge::Top, 10);
    NodeStyleSetMargin(root, Edge::Right, 10);
    NodeStyleSetMargin(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 700);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(130 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(170 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(130 == NodeLayoutGetLeft(root_child3));
    REQUIRE(170 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(340 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(170 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(170 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(200 == NodeLayoutGetLeft(root_child4));
    REQUIRE(340 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_content_margin_and_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetMargin(root, Edge::Left, 10);
    NodeStyleSetMargin(root, Edge::Top, 10);
    NodeStyleSetMargin(root, Edge::Right, 10);
    NodeStyleSetMargin(root, Edge::Bottom, 10);
    NodeStyleSetPadding(root, Edge::Left, 10);
    NodeStyleSetPadding(root, Edge::Top, 10);
    NodeStyleSetPadding(root, Edge::Right, 10);
    NodeStyleSetPadding(root, Edge::Bottom, 10);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 700);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(138 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(138 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(10 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(10 == NodeLayoutGetLeft(root));
    REQUIRE(10 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(190 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(62 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(190 == NodeLayoutGetLeft(root_child2));
    REQUIRE(178 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(62 == NodeLayoutGetLeft(root_child3));
    REQUIRE(178 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(190 == NodeLayoutGetLeft(root_child4));
    REQUIRE(346 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_flexible_content", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child1));
    REQUIRE(300 == NodeLayoutGetHeight(root_child1));

    REQUIRE(220 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(80 == NodeLayoutGetWidth(root_child2));
    REQUIRE(300 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(220 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(110 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child1));
    REQUIRE(300 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(80 == NodeLayoutGetWidth(root_child2));
    REQUIRE(300 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_mixed_flexible_content", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 300);
    NodeStyleSetHeight(root, 300);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidthPercent(root_child2, 10);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(300 == NodeLayoutGetHeight(root_child1));

    REQUIRE(270 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(300 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(290 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(300 == NodeLayoutGetHeight(root_child0));

    REQUIRE(60 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(300 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(300 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::row_gap_percent_wrapping_with_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetMinWidth(root, 300);
    NodeStyleSetGapPercent(root, Gutter::Column, 10);
    NodeStyleSetGapPercent(root, Gutter::Row, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 100);
    NodeStyleSetHeight(root_child4, 100);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(130 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(130 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(0 == NodeLayoutGetLeft(root_child4));
    REQUIRE(200 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(300 == NodeLayoutGetHeight(root));

    REQUIRE(200 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(70 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    REQUIRE(200 == NodeLayoutGetLeft(root_child4));
    REQUIRE(200 == NodeLayoutGetTop(root_child4));
    REQUIRE(100 == NodeLayoutGetWidth(root_child4));
    REQUIRE(100 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
