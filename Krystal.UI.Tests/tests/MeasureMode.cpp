#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  struct _MeasureConstraint
  {
    float width;
    MeasureMode widthMode;
    float height;
    MeasureMode heightMode;
  };

  struct _MeasureConstraintList
  {
    uint32_t length;
    struct _MeasureConstraint *constraints;
  };

  static Size _measure(NodeConstRef node, float width, MeasureMode widthMode, float height,
                       MeasureMode heightMode)
  {
    auto *constraintList = (struct _MeasureConstraintList *)NodeGetContext(node);
    struct _MeasureConstraint *constraints = constraintList->constraints;
    uint32_t currentIndex = constraintList->length;
    (&constraints[currentIndex])->width = width;
    (&constraints[currentIndex])->widthMode = widthMode;
    (&constraints[currentIndex])->height = height;
    (&constraints[currentIndex])->heightMode = heightMode;
    constraintList->length = currentIndex + 1;

    return Size {
      widthMode == MeasureMode::Undefined ? 10 : width,
      heightMode == MeasureMode::Undefined ? 10 : width,
    };
  }

  TEST_CASE("LayoutEngine::exactly_measure_stretched_child_column", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].width);
    REQUIRE(MeasureMode::Exactly == constraintList.constraints[0].widthMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::exactly_measure_stretched_child_row", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::Exactly == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::at_most_main_axis_column", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::at_most_cross_axis_column", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].width);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].widthMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::at_most_main_axis_row", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].width);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].widthMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::at_most_cross_axis_row", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::flex_child", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(2u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].heightMode);

    REQUIRE(100 == constraintList.constraints[1].height);
    REQUIRE(MeasureMode::Exactly == constraintList.constraints[1].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::flex_child_with_flex_basis", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeStyleSetFlexBasis(root_child0, 0);
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::Exactly == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::overflow_scroll_column", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetOverflow(root, Overflow::Scroll);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(100 == constraintList.constraints[0].width);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].widthMode);

    REQUIRE(Krys::IsUndefined(constraintList.constraints[0].height));
    REQUIRE(MeasureMode::Undefined == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::overflow_scroll_row", "[LayoutEngine]")
  {
    auto constraintList = _MeasureConstraintList {
      0,
      (struct _MeasureConstraint *)malloc(10 * sizeof(struct _MeasureConstraint)),
    };

    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetOverflow(root, Overflow::Scroll);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetWidth(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeSetContext(root_child0, &constraintList);
    NodeSetMeasureFunc(root_child0, _measure);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(1u == constraintList.length);

    REQUIRE(Krys::IsUndefined(constraintList.constraints[0].width));
    REQUIRE(MeasureMode::Undefined == constraintList.constraints[0].widthMode);

    REQUIRE(100 == constraintList.constraints[0].height);
    REQUIRE(MeasureMode::AtMost == constraintList.constraints[0].heightMode);

    free(constraintList.constraints);
    NodeDestroyRecursive(root);
  }
}