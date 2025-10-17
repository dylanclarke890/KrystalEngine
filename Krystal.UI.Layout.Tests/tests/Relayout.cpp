#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  TEST_CASE("LayoutEngine::dont_cache_computed_flex_basis_between_layouts", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetExperimentalFeatureEnabled(config, ExperimentalFeature::WebFlexBasis, true);

    NodeRef root = NodeCreate(config);
    NodeStyleSetHeightPercent(root, 100);
    NodeStyleSetWidthPercent(root, 100);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexBasisPercent(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, 100, Undefined, Direction::LTR);
    NodeCalculateLayout(root, 100, 100, Direction::LTR);

    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::recalculate_resolvedDimension_onchange", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetMinHeight(root_child0, 10);
    NodeStyleSetMaxHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeStyleSetMinHeight(root_child0, Undefined);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::relayout_containing_block_size_changes", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0, PositionType::Relative);
    NodeStyleSetMargin(root_child0, Edge::Left, 4);
    NodeStyleSetMargin(root_child0, Edge::Top, 5);
    NodeStyleSetMargin(root_child0, Edge::Right, 9);
    NodeStyleSetMargin(root_child0, Edge::Bottom, 1);
    NodeStyleSetPadding(root_child0, Edge::Left, 2);
    NodeStyleSetPadding(root_child0, Edge::Top, 9);
    NodeStyleSetPadding(root_child0, Edge::Right, 11);
    NodeStyleSetPadding(root_child0, Edge::Bottom, 13);
    NodeStyleSetBorder(root_child0, Edge::Left, 5);
    NodeStyleSetBorder(root_child0, Edge::Top, 6);
    NodeStyleSetBorder(root_child0, Edge::Right, 7);
    NodeStyleSetBorder(root_child0, Edge::Bottom, 8);
    NodeStyleSetWidth(root_child0, 500);
    NodeStyleSetHeight(root_child0, 500);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetMargin(root_child0_child0, Edge::Left, 8);
    NodeStyleSetMargin(root_child0_child0, Edge::Top, 6);
    NodeStyleSetMargin(root_child0_child0, Edge::Right, 3);
    NodeStyleSetMargin(root_child0_child0, Edge::Bottom, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Left, 1);
    NodeStyleSetPadding(root_child0_child0, Edge::Top, 7);
    NodeStyleSetPadding(root_child0_child0, Edge::Right, 9);
    NodeStyleSetPadding(root_child0_child0, Edge::Bottom, 4);
    NodeStyleSetBorder(root_child0_child0, Edge::Left, 8);
    NodeStyleSetBorder(root_child0_child0, Edge::Top, 10);
    NodeStyleSetBorder(root_child0_child0, Edge::Right, 2);
    NodeStyleSetBorder(root_child0_child0, Edge::Bottom, 1);
    NodeStyleSetWidth(root_child0_child0, 200);
    NodeStyleSetHeight(root_child0_child0, 200);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetPosition(root_child0_child0_child0, Edge::Right, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Left, 9);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Top, 12);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Right, 4);
    NodeStyleSetMargin(root_child0_child0_child0, Edge::Bottom, 7);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Left, 5);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Top, 3);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Right, 8);
    NodeStyleSetPadding(root_child0_child0_child0, Edge::Bottom, 10);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Left, 2);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Top, 1);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Right, 5);
    NodeStyleSetBorder(root_child0_child0_child0, Edge::Bottom, 9);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 41);
    NodeStyleSetHeightPercent(root_child0_child0_child0, 63);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(1 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(513 == NodeLayoutGetWidth(root));
    REQUIRE(506 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(279 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-2 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(306 == NodeLayoutGetHeight(root_child0_child0_child0));

    // Relayout starts here
    NodeStyleSetWidth(root_child0, 456);
    NodeStyleSetHeight(root_child0, 432);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(469 == NodeLayoutGetWidth(root));
    REQUIRE(438 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(456 == NodeLayoutGetWidth(root_child0));
    REQUIRE(432 == NodeLayoutGetHeight(root_child0));

    REQUIRE(15 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(1 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(182 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(263 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(469 == NodeLayoutGetWidth(root));
    REQUIRE(438 == NodeLayoutGetHeight(root));

    REQUIRE(4 == NodeLayoutGetLeft(root_child0));
    REQUIRE(5 == NodeLayoutGetTop(root_child0));
    REQUIRE(456 == NodeLayoutGetWidth(root_child0));
    REQUIRE(432 == NodeLayoutGetHeight(root_child0));

    REQUIRE(235 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(16 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(29 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(182 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(263 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::has_new_layout_flag_set_static", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0, 10);
    NodeStyleSetHeight(root_child0, 10);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child1 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child1, PositionType::Absolute);
    NodeStyleSetWidth(root_child0_child1, 5);
    NodeStyleSetHeight(root_child0_child1, 5);
    NodeInsertChild(root_child0, root_child0_child1, 0);

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0, PositionType::Static);
    NodeStyleSetWidth(root_child0_child0, 5);
    NodeStyleSetHeight(root_child0_child0, 5);
    NodeInsertChild(root_child0, root_child0_child0, 1);

    NodeRef root_child0_child0_child0 = NodeCreate();
    NodeStyleSetPositionType(root_child0_child0_child0, PositionType::Absolute);
    NodeStyleSetWidthPercent(root_child0_child0_child0, 1);
    NodeStyleSetHeight(root_child0_child0_child0, 1);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    NodeSetHasNewLayout(root, false);
    NodeSetHasNewLayout(root_child0, false);
    NodeSetHasNewLayout(root_child0_child0, false);
    NodeSetHasNewLayout(root_child0_child0_child0, false);

    NodeStyleSetWidth(root, 110);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(NodeGetHasNewLayout(root));
    REQUIRE(NodeGetHasNewLayout(root_child0));
    REQUIRE(NodeGetHasNewLayout(root_child0_child0));
    REQUIRE(NodeGetHasNewLayout(root_child0_child0_child0));

    NodeDestroyRecursive(root);
  }

  static Size _measureRecalc(NodeConstRef node, float width, MeasureMode widthMode, float height,
                             MeasureMode heightMode)
  {
    return Size {0, 0};
  }

  TEST_CASE("LayoutEngine::recalculate_on_layout_values_change", "[LayoutEngine]")
  {
    SKIP();

    const ConfigRef config = ConfigCreate();
    ConfigSetExperimentalFeatureEnabled(config, ExperimentalFeature::WebFlexBasis, true);
    ConfigSetPointScaleFactor(config, 3.f);

    const NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetAlignContent(root, Align::FlexStart);

    const NodeRef child = NodeCreate(config);
    NodeStyleSetMinHeightPercent(child, 40.f);
    NodeStyleSetMaxHeightPercent(child, 60.f);
    NodeStyleSetHeight(child, 10.f);
    NodeStyleSetWidth(child, 50.0f);
    NodeSetMeasureFunc(child, _measureRecalc);
    NodeInsertChild(root, child, 0);

    NodeCalculateLayout(root, 50.0f, Undefined, Direction::LTR);
    REQUIRE(10 == NodeLayoutGetHeight(child));

    NodeCalculateLayout(root, 50.0f, 30.0f, Direction::LTR);
    REQUIRE(12 == NodeLayoutGetHeight(child));

    NodeDestroyRecursive(root);
  }
}