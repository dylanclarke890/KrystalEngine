#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  static Size _measure(NodeConstRef node, float /*width*/, MeasureMode /*widthMode*/, float /*height*/,
                       MeasureMode /*heightMode*/)
  {
    int *measureCount = (int *)NodeGetContext(node);
    if (measureCount != nullptr)
    {
      (*measureCount)++;
    }

    return Size {10, 10};
  }

  static Size _simulate_wrapping_text(NodeConstRef /*node*/, float width, MeasureMode widthMode,
                                      float /*height*/, MeasureMode /*heightMode*/)
  {
    if (widthMode == MeasureMode::Undefined || width >= 68)
    {
      return Size {68, 16};
    }

    return Size {50, 32};
  }

  static Size _measure_assert_negative(NodeConstRef /*node*/, float width, MeasureMode /*widthMode*/,
                                       float height, MeasureMode /*heightMode*/)
  {
    CHECK(width >= 0);
    CHECK(height >= 0);

    return Size {0, 0};
  }

  TEST_CASE("LayoutEngine::dont_measure_single_grow_shrink_child", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_absolute_child_with_no_constraints", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeInsertChild(root, root_child0, 0);

    int measureCount = 0;

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0, PositionType::Absolute);
    NodeSetContext(root_child0_child0, &measureCount);
    NodeSetMeasureFunc(root_child0_child0, _measure);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dont_measure_when_min_equals_max", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetMinWidth(root_child0, 10);
    NodeStyleSetMaxWidth(root_child0, 10);
    NodeStyleSetMinHeight(root_child0, 10);
    NodeStyleSetMaxHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == measureCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dont_measure_when_min_equals_max_percentages", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetMinWidthPercent(root_child0, 10);
    NodeStyleSetMaxWidthPercent(root_child0, 10);
    NodeStyleSetMinHeightPercent(root_child0, 10);
    NodeStyleSetMaxHeightPercent(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == measureCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_nodes_with_margin_auto_and_stretch", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate();
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetMarginAuto(root_child0, Edge::Left);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    CHECK(490 == NodeLayoutGetLeft(root_child0));
    CHECK(0 == NodeLayoutGetTop(root_child0));
    CHECK(10 == NodeLayoutGetWidth(root_child0));
    CHECK(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dont_measure_when_min_equals_max_mixed_width_percent", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetMinWidthPercent(root_child0, 10);
    NodeStyleSetMaxWidthPercent(root_child0, 10);
    NodeStyleSetMinHeight(root_child0, 10);
    NodeStyleSetMaxHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == measureCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dont_measure_when_min_equals_max_mixed_height_percent", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeStyleSetMinWidth(root_child0, 10);
    NodeStyleSetMaxWidth(root_child0, 10);
    NodeStyleSetMinHeightPercent(root_child0, 10);
    NodeStyleSetMaxHeightPercent(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == measureCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_enough_size_should_be_in_single_line", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetAlignSelf(root_child0, Align::FlexStart);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);

    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(68 == NodeLayoutGetWidth(root_child0));
    REQUIRE(16 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_not_enough_size_should_wrap", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 55);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetAlignSelf(root_child0, Align::FlexStart);
    // NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(32 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_zero_space_should_grow", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetHeight(root, 200);
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetFlexGrow(root, 0);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Column);
    NodeStyleSetPadding(root_child0, Edge::All, 100);
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure);

    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 282, Undefined, Direction::LTR);

    REQUIRE(282 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_flex_direction_row_and_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPadding(root, Edge::Left, 25);
    NodeStyleSetPadding(root, Edge::Top, 25);
    NodeStyleSetPadding(root, Edge::Right, 25);
    NodeStyleSetPadding(root, Edge::Bottom, 25);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(75 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_flex_direction_column_and_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::All, 25);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    // NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(32 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(57 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_flex_direction_row_no_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    // NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_flex_direction_row_no_padding_align_items_flexstart", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);
    NodeStyleSetAlignItems(root, Align::FlexStart);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(32 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_with_fixed_size", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::All, 25);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(10 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(35 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_with_flex_shrink", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetPadding(root, Edge::All, 25);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(25 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(25 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::measure_no_padding", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetMargin(root, Edge::Top, 20);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _simulate_wrapping_text);
    NodeStyleSetFlexShrink(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 5);
    NodeStyleSetHeight(root_child1, 5);
    NodeInsertChild(root, root_child1, 1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(20 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(32 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(32 == NodeLayoutGetTop(root_child1));
    REQUIRE(5 == NodeLayoutGetWidth(root_child1));
    REQUIRE(5 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

#if GTEST_HAS_DEATH_TEST
  TEST(YogaDeathTest, cannot_add_child_to_node_with_measure_func)
  {
    NodeRef root = NodeCreate();
    NodeSetMeasureFunc(root, _measure);

    NodeRef root_child0 = NodeCreate();
  #if defined(__cpp_exceptions)
    ASSERT_THROW(NodeInsertChild(root, root_child0, 0), std::logic_error);
  #else  // !defined(__cpp_exceptions)
    ASSERT_DEATH(NodeInsertChild(root, root_child0, 0), "Cannot add child.*");
  #endif // defined(__cpp_exceptions)
    NodeDestroy(root_child0);
    NodeDestroyRecursive(root);
  }

  TEST(YogaDeathTest, cannot_add_nonnull_measure_func_to_non_leaf_node)
  {
    NodeRef root = NodeCreate();
    NodeRef root_child0 = NodeCreate();
    NodeInsertChild(root, root_child0, 0);
  #if defined(__cpp_exceptions)
    ASSERT_THROW(NodeSetMeasureFunc(root, _measure), std::logic_error);
  #else  // !defined(__cpp_exceptions)
    ASSERT_DEATH(NodeSetMeasureFunc(root, _measure), "Cannot set measure function.*");
  #endif // defined(__cpp_exceptions)
    NodeDestroyRecursive(root);
  }

#endif

  TEST_CASE("LayoutEngine::can_nullify_measure_func_on_any_node", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeInsertChild(root, NodeCreate(), 0);
    NodeSetMeasureFunc(root, nullptr);
    REQUIRE(!NodeHasMeasureFunc(root));
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::cant_call_negative_measure", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Column);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 10);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _measure_assert_negative);
    NodeStyleSetMargin(root_child0, Edge::Top, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::cant_call_negative_measure_horizontal", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 10);
    NodeStyleSetHeight(root, 20);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _measure_assert_negative);
    NodeStyleSetMargin(root_child0, Edge::Start, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
  }

  static Size _measure_90_10(NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/,
                             float /*height*/, MeasureMode /*heightMode*/)
  {
    return Size {90, 10};
  }

  static Size _measure_100_100(NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/,
                               float /*height*/, MeasureMode /*heightMode*/)
  {
    return Size {100, 100};
  }

  TEST_CASE("LayoutEngine::percent_with_text_node", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetJustifyContent(root, Justify::SpaceBetween);
    NodeStyleSetAlignItems(root, Align::Center);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 80);

    NodeRef root_child0 = NodeCreate(config);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeSetMeasureFunc(root_child1, _measure_90_10);
    NodeStyleSetMaxWidthPercent(root_child1, 50);
    NodeStyleSetPaddingPercent(root_child1, Edge::Top, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(80 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(40 == NodeLayoutGetTop(root_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(10 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(60 == NodeLayoutGetHeight(root_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::percent_margin_with_measure_func", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeStyleSetMargin(root_child0, Edge::Top, 0);
    NodeSetMeasureFunc(root_child0, _measure_100_100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeStyleSetMargin(root_child1, Edge::Top, 100);
    NodeSetMeasureFunc(root_child1, _measure_100_100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 100);
    NodeStyleSetHeight(root_child2, 100);
    NodeStyleSetMarginPercent(root_child2, Edge::Top, 10);
    NodeSetMeasureFunc(root_child2, _measure_100_100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetWidth(root_child3, 100);
    NodeStyleSetHeight(root_child3, 100);
    NodeStyleSetMarginPercent(root_child3, Edge::Top, 20);
    NodeSetMeasureFunc(root_child3, _measure_100_100);
    NodeInsertChild(root, root_child3, 3);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(100 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(100 == NodeLayoutGetHeight(root_child2));

    REQUIRE(300 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(100 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::percent_padding_with_measure_func", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetAlignContent(root, Align::FlexStart);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 0);
    NodeSetMeasureFunc(root_child0, _measure_100_100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeStyleSetPadding(root_child1, Edge::Top, 100);
    NodeSetMeasureFunc(root_child1, _measure_100_100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child2, Edge::Top, 10);
    NodeSetMeasureFunc(root_child2, _measure_100_100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child3, Edge::Top, 20);
    NodeSetMeasureFunc(root_child3, _measure_100_100);
    NodeInsertChild(root, root_child3, 3);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(150 == NodeLayoutGetHeight(root_child2));

    REQUIRE(300 == NodeLayoutGetLeft(root_child3));
    REQUIRE(0 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(200 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::percent_padding_and_percent_margin_with_measure_func", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetAlignContent(root, Align::FlexStart);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetHeight(root_child0, 100);
    NodeStyleSetPadding(root_child0, Edge::Top, 0);
    NodeSetMeasureFunc(root_child0, _measure_100_100);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 100);
    NodeStyleSetPadding(root_child1, Edge::Top, 100);
    NodeSetMeasureFunc(root_child1, _measure_100_100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child2, Edge::Top, 10);
    NodeStyleSetMarginPercent(root_child2, Edge::Top, 10);
    NodeSetMeasureFunc(root_child2, _measure_100_100);
    NodeInsertChild(root, root_child2, 2);

    NodeRef root_child3 = NodeCreate(config);
    NodeStyleSetPaddingPercent(root_child3, Edge::Top, 20);
    NodeStyleSetMarginPercent(root_child3, Edge::Top, 20);
    NodeSetMeasureFunc(root_child3, _measure_100_100);
    NodeInsertChild(root, root_child3, 3);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(100 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(200 == NodeLayoutGetLeft(root_child2));
    REQUIRE(50 == NodeLayoutGetTop(root_child2));
    REQUIRE(100 == NodeLayoutGetWidth(root_child2));
    REQUIRE(150 == NodeLayoutGetHeight(root_child2));

    REQUIRE(300 == NodeLayoutGetLeft(root_child3));
    REQUIRE(100 == NodeLayoutGetTop(root_child3));
    REQUIRE(100 == NodeLayoutGetWidth(root_child3));
    REQUIRE(200 == NodeLayoutGetHeight(root_child3));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  static Size _measure_half_width_height(NodeConstRef node, float width, MeasureMode /*widthMode*/,
                                         float height, MeasureMode /*heightMode*/)
  {
    int *measureCount = (int *)NodeGetContext(node);
    if (measureCount != nullptr)
    {
      (*measureCount)++;
    }

    return Size {0.5f * width, 0.5f * height};
  }

  TEST_CASE("LayoutEngine::measure_content_box", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetBoxSizing(root, BoxSizing::ContentBox);
    NodeStyleSetPadding(root, Edge::All, 5);
    NodeStyleSetBorder(root, Edge::All, 10);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure_half_width_height);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1 == measureCount);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(130 == NodeLayoutGetWidth(root));
    REQUIRE(230 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::measure_border_box", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetBoxSizing(root, BoxSizing::BorderBox);
    NodeStyleSetPadding(root, Edge::All, 5);
    NodeStyleSetBorder(root, Edge::All, 10);

    int measureCount = 0;

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measure_half_width_height);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1 == measureCount);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(200 == NodeLayoutGetHeight(root));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0));
    REQUIRE(15 == NodeLayoutGetTop(root_child0));
    REQUIRE(70 == NodeLayoutGetWidth(root_child0));
    REQUIRE(85 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }
}