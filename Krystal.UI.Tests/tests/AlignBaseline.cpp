#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  static float _baselineFunc(NodeConstRef /*node*/, const float /*width*/, const float height)
  {
    return height / 2;
  }

  static Size _measure1(NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/, float /*height*/,
                        MeasureMode /*heightMode*/)
  {
    return Size {42, 50};
  }

  static Size _measure2(NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/, float /*height*/,
                        MeasureMode /*heightMode*/)
  {
    return Size {279, 126};
  }

  static NodeRef CreateNode(ConfigRef config, FlexDirection direction, int width, int height,
                            bool alignBaseline)
  {
    NodeRef node = NodeCreate(config);
    NodeStyleSetFlexDirection(node, direction);
    if (alignBaseline)
    {
      NodeStyleSetAlignItems(node, Align::Baseline);
    }
    NodeStyleSetWidth(node, (float)width);
    NodeStyleSetHeight(node, (float)height);
    return node;
  }

  // Test case for bug in T32999822
  TEST_CASE("LayoutEngine::align_baseline_parent_ht_not_specified", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 340);
    NodeStyleSetMaxHeight(root, 170);
    NodeStyleSetMinHeight(root, 0);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 0);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeSetMeasureFunc(root_child0, _measure1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 0);
    NodeStyleSetFlexShrink(root_child1, 1);
    NodeSetMeasureFunc(root_child1, _measure2);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(340 == NodeLayoutGetWidth(root));
    REQUIRE(126 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(42 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));
    REQUIRE(76 == NodeLayoutGetTop(root_child0));

    REQUIRE(42 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(279 == NodeLayoutGetWidth(root_child1));
    REQUIRE(126 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_with_no_parent_ht", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 150);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 40);
    NodeSetBaselineFunc(root_child1, _baselineFunc);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(70 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(40 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_with_no_baseline_func_and_no_parent_ht", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 150);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 80);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(150 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(80 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(30 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_column_as_reference", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_with_padding_in_column_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeStyleSetPadding(root_child1_child1, Edge::Left, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Right, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Top, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Bottom, 100);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_with_padding_using_child_in_column_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeStyleSetPadding(root_child1, Edge::Left, 100);
    NodeStyleSetPadding(root_child1, Edge::Right, 100);
    NodeStyleSetPadding(root_child1, Edge::Top, 100);
    NodeStyleSetPadding(root_child1, Edge::Bottom, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(400 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_with_margin_using_child_in_column_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeStyleSetMargin(root_child1, Edge::Left, 100);
    NodeStyleSetMargin(root_child1, Edge::Right, 100);
    NodeStyleSetMargin(root_child1, Edge::Top, 100);
    NodeStyleSetMargin(root_child1, Edge::Bottom, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(600 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_with_margin_in_column_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeStyleSetMargin(root_child1_child1, Edge::Left, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Right, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Top, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Bottom, 100);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(400 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_row_as_reference", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Row, 500, 800, true);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_with_padding_in_row_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Row, 500, 800, true);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeStyleSetPadding(root_child1_child1, Edge::Left, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Right, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Top, 100);
    NodeStyleSetPadding(root_child1_child1, Edge::Bottom, 100);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_with_margin_in_row_as_reference",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Row, 500, 800, true);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeStyleSetMargin(root_child1_child1, Edge::Left, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Right, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Top, 100);
    NodeStyleSetMargin(root_child1_child1, Edge::Bottom, 100);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(600 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_column_as_reference_with_no_baseline_func",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Column, 500, 800, false);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_row_as_reference_with_no_baseline_func",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = CreateNode(config, FlexDirection::Row, 1'000, 1'000, true);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = CreateNode(config, FlexDirection::Row, 500, 800, true);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE(
    "LayoutEngine::align_baseline_parent_using_child_in_column_as_reference_with_height_not_specified",
    "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 1'000);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Column);
    NodeStyleSetWidth(root_child1, 500);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(800 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(700 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_row_as_reference_with_height_not_specified",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 1'000);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Row);
    NodeStyleSetWidth(root_child1, 500);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetBaselineFunc(root_child1_child1, _baselineFunc);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(900 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(400 == NodeLayoutGetTop(root_child1));
    REQUIRE(500 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_column_as_reference_with_no_baseline_func_"
            "and_height_not_specified",
            "[LayoutEngine]")

  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 1'000);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Column);
    NodeStyleSetWidth(root_child1, 500);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 300, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(100 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(700 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(300 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::align_baseline_parent_using_child_in_row_as_reference_with_no_baseline_func_and_"
            "height_not_specified",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);
    NodeStyleSetWidth(root, 1'000);

    NodeRef root_child0 = CreateNode(config, FlexDirection::Column, 500, 600, false);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child1, FlexDirection::Row);
    NodeStyleSetWidth(root_child1, 500);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child1_child0 = CreateNode(config, FlexDirection::Column, 500, 500, false);
    NodeInsertChild(root_child1, root_child1_child0, 0);

    NodeRef root_child1_child1 = CreateNode(config, FlexDirection::Column, 500, 400, false);
    NodeSetIsReferenceBaseline(root_child1_child1, true);
    NodeInsertChild(root_child1, root_child1_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(700 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1));
    REQUIRE(200 == NodeLayoutGetTop(root_child1));
    REQUIRE(500 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child0));

    REQUIRE(500 == NodeLayoutGetLeft(root_child1_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}