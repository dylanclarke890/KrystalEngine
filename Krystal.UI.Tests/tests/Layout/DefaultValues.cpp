#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::assert_default_values", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();

    REQUIRE(0u == NodeGetChildCount(root));
    REQUIRE(nullptr == NodeGetChild(root, 1));

    REQUIRE(Direction::Inherit == NodeStyleGetDirection(root));
    REQUIRE(FlexDirection::Column == NodeStyleGetFlexDirection(root));
    REQUIRE(Justify::FlexStart == NodeStyleGetJustifyContent(root));
    REQUIRE(Align::FlexStart == NodeStyleGetAlignContent(root));
    REQUIRE(Align::Stretch == NodeStyleGetAlignItems(root));
    REQUIRE(Align::Auto == NodeStyleGetAlignSelf(root));
    REQUIRE(PositionType::Relative == NodeStyleGetPositionType(root));
    REQUIRE(Wrap::NoWrap == NodeStyleGetFlexWrap(root));
    REQUIRE(Overflow::Visible == NodeStyleGetOverflow(root));
    REQUIRE(0 == NodeStyleGetFlexGrow(root));
    REQUIRE(0 == NodeStyleGetFlexShrink(root));
    REQUIRE(NodeStyleGetFlexBasis(root).Type == Unit::Auto);

    REQUIRE(NodeStyleGetPosition(root, Edge::Left).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPosition(root, Edge::Top).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPosition(root, Edge::Right).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPosition(root, Edge::Bottom).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPosition(root, Edge::Start).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPosition(root, Edge::End).Type == Unit::Undefined);

    REQUIRE(NodeStyleGetMargin(root, Edge::Left).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMargin(root, Edge::Top).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMargin(root, Edge::Right).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMargin(root, Edge::Bottom).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMargin(root, Edge::Start).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMargin(root, Edge::End).Type == Unit::Undefined);

    REQUIRE(NodeStyleGetPadding(root, Edge::Left).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPadding(root, Edge::Top).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPadding(root, Edge::Right).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPadding(root, Edge::Bottom).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPadding(root, Edge::Start).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetPadding(root, Edge::End).Type == Unit::Undefined);

    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::Left)));
    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::Top)));
    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::Right)));
    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::Bottom)));
    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::Start)));
    REQUIRE(Krys::IsUndefined(NodeStyleGetBorder(root, Edge::End)));

    REQUIRE(NodeStyleGetWidth(root).Type == Unit::Auto);
    REQUIRE(NodeStyleGetHeight(root).Type == Unit::Auto);
    REQUIRE(NodeStyleGetMinWidth(root).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMinHeight(root).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMaxWidth(root).Type == Unit::Undefined);
    REQUIRE(NodeStyleGetMaxHeight(root).Type == Unit::Undefined);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(0 == NodeLayoutGetRight(root));
    REQUIRE(0 == NodeLayoutGetBottom(root));

    REQUIRE(0 == NodeLayoutGetMargin(root, Edge::Left));
    REQUIRE(0 == NodeLayoutGetMargin(root, Edge::Top));
    REQUIRE(0 == NodeLayoutGetMargin(root, Edge::Right));
    REQUIRE(0 == NodeLayoutGetMargin(root, Edge::Bottom));

    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Left));
    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Top));
    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Right));
    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Bottom));

    REQUIRE(0 == NodeLayoutGetBorder(root, Edge::Left));
    REQUIRE(0 == NodeLayoutGetBorder(root, Edge::Top));
    REQUIRE(0 == NodeLayoutGetBorder(root, Edge::Right));
    REQUIRE(0 == NodeLayoutGetBorder(root, Edge::Bottom));

    REQUIRE(Krys::IsUndefined(NodeLayoutGetWidth(root)));
    REQUIRE(Krys::IsUndefined(NodeLayoutGetHeight(root)));
    REQUIRE(Direction::Inherit == NodeLayoutGetDirection(root));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::assert_webdefault_values", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetUseWebDefaults(config, true);
    NodeRef root = NodeCreate(config);

    REQUIRE(FlexDirection::Row == NodeStyleGetFlexDirection(root));
    REQUIRE(Align::Stretch == NodeStyleGetAlignContent(root));
    REQUIRE(1.0f == NodeStyleGetFlexShrink(root));

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::assert_webdefault_values_reset", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetUseWebDefaults(config, true);
    NodeRef root = NodeCreate(config);
    NodeReset(root);

    REQUIRE(FlexDirection::Row == NodeStyleGetFlexDirection(root));
    REQUIRE(Align::Stretch == NodeStyleGetAlignContent(root));
    REQUIRE(1.0f == NodeStyleGetFlexShrink(root));

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::assert_legacy_stretch_behaviour", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetErrata(config, Errata::StretchFlexBasis);
    NodeRef root = NodeCreate(config);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetAlignItems(root_child0, Align::FlexStart);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0, 1);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0_child0_child0, 1);
    NodeStyleSetFlexShrink(root_child0_child0_child0, 1);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::assert_box_sizing_border_box", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    NodeRef root = NodeCreate(config);

    REQUIRE(BoxSizing::BorderBox == NodeStyleGetBoxSizing(root));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}