#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  static Size _measureMax(NodeConstRef node, float width, MeasureMode widthMode, float height,
                          MeasureMode heightMode)
  {
    int *measureCount = (int *)NodeGetContext(node);
    (*measureCount)++;

    return Size {
      widthMode == MeasureMode::Undefined ? 10 : width,
      heightMode == MeasureMode::Undefined ? 10 : height,
    };
  }

  static Size _measureMin(NodeConstRef node, float width, MeasureMode widthMode, float height,
                          MeasureMode heightMode)
  {
    int *measureCount = (int *)NodeGetContext(node);
    *measureCount = *measureCount + 1;
    return Size {
      widthMode == MeasureMode::Undefined || (widthMode == MeasureMode::AtMost && width > 10) ? 10 : width,
      heightMode == MeasureMode::Undefined || (heightMode == MeasureMode::AtMost && height > 10) ? 10
                                                                                                 : height,
    };
  }

  static Size _measure_84_49(NodeConstRef node, float /*width*/, MeasureMode /*widthMode*/, float /*height*/,
                             MeasureMode /*heightMode*/)
  {
    int *measureCount = (int *)NodeGetContext(node);
    if (measureCount != nullptr)
    {
      (*measureCount)++;
    }

    return Size {84.f, 49.f};
  }

  TEST_CASE("LayoutEngine::measure_once_single_flexible_child", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    int measureCount = 0;
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measureMax);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::remeasure_with_same_exact_width_larger_than_needed_height", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    int measureCount = 0;
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measureMin);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);
    NodeCalculateLayout(root, 100, 50, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::remeasure_with_same_atmost_width_larger_than_needed_height", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);

    NodeRef root_child0 = NodeCreate();
    int measureCount = 0;
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measureMin);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);
    NodeCalculateLayout(root, 100, 50, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::remeasure_with_computed_width_larger_than_needed_height", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);

    NodeRef root_child0 = NodeCreate();
    int measureCount = 0;
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measureMin);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);
    NodeStyleSetAlignItems(root, Align::Stretch);
    NodeCalculateLayout(root, 10, 50, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::remeasure_with_atmost_computed_width_undefined_height", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);

    NodeRef root_child0 = NodeCreate();
    int measureCount = 0;
    NodeSetContext(root_child0, &measureCount);
    NodeSetMeasureFunc(root_child0, _measureMin);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 100, Undefined, Direction::LTR);
    NodeCalculateLayout(root, 10, Undefined, Direction::LTR);

    REQUIRE(1 == measureCount);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::remeasure_with_already_measured_value_smaller_but_still_float_equal",
            "[LayoutEngine]")
  {
    int measureCount = 0;

    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 288.f);
    NodeStyleSetHeight(root, 288.f);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPadding(root_child0, Edge::All, 2.88f);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate();
    NodeSetContext(root_child0_child0, &measureCount);
    NodeSetMeasureFunc(root_child0_child0, _measure_84_49);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeDestroyRecursive(root);

    REQUIRE(1 == measureCount);
  }
}