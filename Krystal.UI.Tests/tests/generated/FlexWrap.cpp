#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::wrap_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 30);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 30);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 30);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(60 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(30 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(60 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1));

    REQUIRE(30 == NodeLayoutGetLeft(root_child2));
    REQUIRE(60 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 30);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 30);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 30);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(30 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_row_align_items_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 30);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(20 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_row_align_items_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 30);
    NodeInsertChild(root, root_child3, 3);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(5 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(60 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(5 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(30 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(30 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_wrap_children_with_min_main_overriding_flex_basis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetMinWidth(root_child0, 55);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexBasis(root_child1, 50);
    NodeStyleSetMinWidth(root_child1, 55);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(55 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(55 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(45 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(55 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(45 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(55 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_wrap_wrap_to_child_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetAlignItems(root_child0, Align::FlexStart);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child0, 100);
    NodeStyleSetHeight(root_child0_child0_child0, 100);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::flex_wrap_align_stretch_fits_one_row", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
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
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_row_align_content_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(40 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_row_align_content_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(40 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_row_single_line_different_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 300);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(40 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(90 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(120 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(300 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(270 == NodeLayoutGetLeft(root_child0));
    REQUIRE(40 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(240 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(210 == NodeLayoutGetLeft(root_child2));
    REQUIRE(20 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(180 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(150 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_row_align_content_stretch", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(40 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_row_align_content_space_around", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::SpaceAround);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(60 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(70 == NodeLayoutGetLeft(root_child0));
    REQUIRE(70 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child1));
    REQUIRE(60 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(10 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(70 == NodeLayoutGetLeft(root_child3));
    REQUIRE(10 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(40 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_reverse_column_fixed_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::WrapReverse);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 30);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 30);
    NodeStyleSetHeight(root_child2, 30);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 30);
    NodeStyleSetHeight(root_child3, 40);
    NodeInsertChild(root, root_child3, 3);

    NodeRef root_child4 = NodeCreate(config);
    NodeStyleSetWidth(root_child4, 30);
    NodeStyleSetHeight(root_child4, 50);
    NodeInsertChild(root, root_child4, 4);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(170 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(170 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(170 == NodeLayoutGetLeft(root_child2));
    REQUIRE(30 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(170 == NodeLayoutGetLeft(root_child3));
    REQUIRE(60 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(140 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(30 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(30 == NodeLayoutGetWidth(root_child1));
    REQUIRE(20 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(30 == NodeLayoutGetTop(root_child2));
    REQUIRE(30 == NodeLayoutGetWidth(root_child2));
    REQUIRE(30 == NodeLayoutGetHeight(root_child2));

    REQUIRE(0 == NodeLayoutGetLeft(root_child3));
    REQUIRE(60 == NodeLayoutGetTop(root_child3));
    REQUIRE(30 == NodeLayoutGetWidth(root_child3));
    REQUIRE(40 == NodeLayoutGetHeight(root_child3));

    REQUIRE(30 == NodeLayoutGetLeft(root_child4));
    REQUIRE(0 == NodeLayoutGetTop(root_child4));
    REQUIRE(30 == NodeLayoutGetWidth(root_child4));
    REQUIRE(50 == NodeLayoutGetHeight(root_child4));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrapped_row_within_align_items_center", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 150);
    NodeStyleSetHeight(root_child0_child0, 80);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 80);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(120 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrapped_row_within_align_items_flex_start", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 150);
    NodeStyleSetHeight(root_child0_child0, 80);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 80);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(120 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrapped_row_within_align_items_flex_end", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexEnd);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 150);
    NodeStyleSetHeight(root_child0_child0, 80);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 80);
    NodeStyleSetHeight(root_child0_child1, 80);
    NodeInsertChild(root_child0, root_child0_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(160 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(150 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(120 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrapped_column_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 700);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 500);
    NodeStyleSetMaxHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child1, Edge::Left, 20);
    NodeStyleSetMargin(root_child1, Edge::Top, 20);
    NodeStyleSetMargin(root_child1, Edge::Right, 20);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 20);
    NodeStyleSetWidth(root_child1, 200);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(700 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(250 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetLeft(root_child1));
    REQUIRE(250 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    REQUIRE(420 == NodeLayoutGetLeft(root_child2));
    REQUIRE(200 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(700 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(350 == NodeLayoutGetLeft(root_child0));
    REQUIRE(30 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    REQUIRE(300 == NodeLayoutGetLeft(root_child1));
    REQUIRE(250 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    REQUIRE(180 == NodeLayoutGetLeft(root_child2));
    REQUIRE(200 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrapped_column_max_height_flex", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetAlignContent(root, Align::Center);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 700);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 500);
    NodeStyleSetMaxHeight(root_child0, 200);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetFlexBasisPercent(root_child1, 0);
    NodeStyleSetMargin(root_child1, Edge::Left, 20);
    NodeStyleSetMargin(root_child1, Edge::Top, 20);
    NodeStyleSetMargin(root_child1, Edge::Right, 20);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 20);
    NodeStyleSetWidth(root_child1, 200);
    NodeStyleSetHeight(root_child1, 200);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(700 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(300 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(180 == NodeLayoutGetHeight(root_child0));

    REQUIRE(250 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(180 == NodeLayoutGetHeight(root_child1));

    REQUIRE(300 == NodeLayoutGetLeft(root_child2));
    REQUIRE(400 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(700 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(300 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(180 == NodeLayoutGetHeight(root_child0));

    REQUIRE(250 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(200 == NodeLayoutGetWidth(root_child1));
    REQUIRE(180 == NodeLayoutGetHeight(root_child1));

    REQUIRE(300 == NodeLayoutGetLeft(root_child2));
    REQUIRE(400 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_nodes_with_content_sizing_overflowing_margin", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 85);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child0, 40);
    NodeStyleSetHeight(root_child0_child0_child0, 40);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child1, Edge::Right, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1_child0, 40);
    NodeStyleSetHeight(root_child0_child1_child0, 40);
    NodeInsertChild(root_child0_child1, root_child0_child1_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(85 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(415 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(85 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    REQUIRE(45 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(35 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_nodes_with_content_sizing_margin_cross", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 70);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0_child0, 40);
    NodeStyleSetHeight(root_child0_child0_child0, 40);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetMargin(root_child0_child1, Edge::Top, 10);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1_child0, 40);
    NodeStyleSetHeight(root_child0_child1_child0, 40);
    NodeInsertChild(root_child0_child1, root_child0_child1_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(70 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(430 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(70 == NodeLayoutGetWidth(root_child0));
    REQUIRE(90 == NodeLayoutGetHeight(root_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(30 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0_child1_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0_child1_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_with_min_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
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
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
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
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(400 == NodeLayoutGetWidth(root_child1));
    REQUIRE(200 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_with_max_cross_axis", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
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

  TEST_CASE("LayoutEngine::nowrap_expands_flexline_box_to_min_cross", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetMinHeight(root, 400);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(0 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(0 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(400 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::wrap_does_not_impose_min_cross_onto_single_flexline", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetMinHeight(root, 400);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetFlexBasisPercent(root_child0, 0);
    NodeInsertChild(root, root_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(0 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(0 == NodeLayoutGetWidth(root));
    REQUIRE(400 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
