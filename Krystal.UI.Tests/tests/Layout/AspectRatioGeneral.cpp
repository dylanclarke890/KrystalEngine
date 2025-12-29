#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  static Size _measure(NodeConstRef /*node*/, float width, MeasureMode widthMode, float height,
                       MeasureMode heightMode)
  {
    return Size {
      widthMode == MeasureMode::Exactly ? width : 50,
      heightMode == MeasureMode::Exactly ? height : 50,
    };
  }

  TEST_CASE("LayoutEngine::aspect_ratio_cross_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_main_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_both_dimensions_defined_row", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_both_dimensions_defined_column", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_align_stretch", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_flex_grow", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_flex_shrink", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 150);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_flex_shrink_2", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeightPercent(root_child0, 100);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetHeightPercent(root_child1, 100);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeStyleSetAspectRatio(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_basis", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexBasis(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_absolute_layout_width_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 0);
    NodeStyleSetPosition(root_child0, Edge::Top, 0);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_absolute_layout_height_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 0);
    NodeStyleSetPosition(root_child0, Edge::Top, 0);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_with_max_cross_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetMaxWidth(root_child0, 40);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_with_max_main_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetMaxHeight(root_child0, 40);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_with_min_cross_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 30);
    NodeStyleSetMinWidth(root_child0, 40);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_with_min_main_defined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 30);
    NodeStyleSetMinHeight(root_child0, 40);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(40 == NodeLayoutGetWidth(root_child0));
    REQUIRE(40 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_double_cross", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 2);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_half_cross", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 100);
    NodeStyleSetAspectRatio(root_child0, 0.5);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_double_main", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 0.5);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_half_main", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetAspectRatio(root_child0, 2);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_with_measure_func", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_width_height_flex_grow_row", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_width_height_flex_grow_column", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_height_as_flex_basis", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetHeight(root_child1, 100);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetAspectRatio(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(75 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(125 == NodeLayoutGetWidth(root_child1));
    REQUIRE(125 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_width_as_flex_basis", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetHeight(root, 200);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeStyleSetAspectRatio(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(75 == NodeLayoutGetWidth(root_child0));
    REQUIRE(75 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(75 == NodeLayoutGetTop(root_child1));
    REQUIRE(125 == NodeLayoutGetWidth(root_child1));
    REQUIRE(125 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_overrides_flex_grow_row", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 0.5);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_overrides_flex_grow_column", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetAspectRatio(root_child0, 2);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_left_right_absolute", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 10);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetPosition(root_child0, Edge::Right, 10);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_top_bottom_absolute", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0, Edge::Left, 10);
    NodeStyleSetPosition(root_child0, Edge::Top, 10);
    NodeStyleSetPosition(root_child0, Edge::Bottom, 10);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(10 == NodeLayoutGetTop(root_child0));
    REQUIRE(80 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_width_overrides_align_stretch_row", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_height_overrides_align_stretch_column", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_allow_child_overflow_parent_size", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 4);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_defined_main_with_margin", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetHeight(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_defined_cross_with_margin", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Left, 10);
    NodeStyleSetMargin(root_child0, Edge::Right, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_defined_cross_with_main_margin", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetJustifyContent(root, Justify::Center);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetAspectRatio(root_child0, 1);
    NodeStyleSetMargin(root_child0, Edge::Top, 10);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_should_prefer_explicit_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetUseWebDefaults(config, true);

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Column);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Column);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Column);
    NodeStyleSetHeight(root_child0_child0, 100);
    NodeStyleSetAspectRatio(root_child0_child0, 2);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, 100, 200, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_should_prefer_explicit_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetUseWebDefaults(config, true);

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0_child0, 100);
    NodeStyleSetAspectRatio(root_child0_child0, 0.5);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, 200, 100, Direction::LTR);

    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::aspect_ratio_should_prefer_flexed_dimension", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetUseWebDefaults(config, true);

    NodeRef root = NodeCreate(config);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Column);
    NodeStyleSetAspectRatio(root_child0, 2);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAspectRatio(root_child0_child0, 4);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(100 == NodeLayoutGetHeight(root));

    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);
  }
}