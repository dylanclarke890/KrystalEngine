#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::display_none", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetDisplay(root_child1, Display::None);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_none_fixed_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 20);
    NodeStyleSetHeight(root_child1, 20);
    NodeStyleSetDisplay(root_child1, Display::None);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_none_with_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeStyleSetDisplay(root_child0, Display::None);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_none_with_child", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetDisplay(root_child1, Display::None);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1_child0, 1);
    NodeStyleSetFlexShrink(root_child1_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child1_child0, 0);
    NodeStyleSetWidth(root_child1_child0, 20);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_none_with_position", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetPosition(root_child1, Edge::Top, 10);
    NodeStyleSetDisplay(root_child1, Display::None);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_none_with_position_absolute", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeStyleSetDisplay(root_child0, Display::None);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_fixed_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_with_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeStyleSetWidth(root_child0, 20);
    NodeStyleSetHeight(root_child0, 20);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_with_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPadding(root_child0, Edge::Left, 10);
    NodeStyleSetPadding(root_child0, Edge::Top, 10);
    NodeStyleSetPadding(root_child0, Edge::Right, 10);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 10);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_with_position", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_with_position_absolute", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0, 10);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child1, 20);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_nested", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetDisplay(root_child0, Display::Contents);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetDisplay(root_child0_child0, Display::Contents);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child0, 0);
    NodeStyleSetHeight(root_child0_child0_child0, 10);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child1, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child0_child0_child1, 0);
    NodeStyleSetHeight(root_child0_child0_child1, 20);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::display_contents_with_siblings", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeStyleSetHeight(root_child0, 30);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetDisplay(root_child1, Display::Contents);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1_child0, 1);
    NodeStyleSetFlexShrink(root_child1_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child1_child0, 0);
    NodeStyleSetHeight(root_child1_child0, 10);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1_child1, 1);
    NodeStyleSetFlexShrink(root_child1_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1_child1, 0);
    NodeStyleSetHeight(root_child1_child1, 20);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child2, 1);
    NodeStyleSetFlexShrink(root_child2, 1);
    NodeStyleSetFlexBasisPercent(root_child2, 0);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child1_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));
    REQUIRE(25 == NodeLayoutGetWidth(root_child1_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
