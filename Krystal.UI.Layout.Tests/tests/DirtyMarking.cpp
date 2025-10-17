#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;

  TEST_CASE("LayoutEngine::dirty_propagation", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeStyleSetWidth(root_child0, 20);

    REQUIRE(NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(NodeIsDirty(root));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_propagation_only_if_prop_changed", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeStyleSetWidth(root_child0, 50);

    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_propagation_changing_layout_config", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0_child0, 25);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root, root_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(!NodeIsDirty(root));
    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root_child0_child0));

    ConfigRef newConfig = ConfigCreate();
    ConfigSetErrata(newConfig, Errata::StretchFlexBasis);
    NodeSetConfig(root_child0, newConfig);

    REQUIRE(NodeIsDirty(root));
    REQUIRE(NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(!NodeIsDirty(root));
    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root_child0_child0));

    ConfigDestroy(newConfig);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_propagation_changing_benign_config", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);

    NodeRef root_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate();
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 20);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child0_child0 = NodeCreate();
    NodeStyleSetWidth(root_child0_child0, 25);
    NodeStyleSetHeight(root_child0_child0, 20);
    NodeInsertChild(root, root_child0_child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(!NodeIsDirty(root));
    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root_child0_child0));

    ConfigRef newConfig = ConfigCreate();
    NodeSetConfig(root_child0, newConfig);

    REQUIRE(!NodeIsDirty(root));
    REQUIRE(!NodeIsDirty(root_child0));
    REQUIRE(!NodeIsDirty(root_child1));
    REQUIRE(!NodeIsDirty(root_child0_child0));

    ConfigDestroy(newConfig);
    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_mark_all_children_as_dirty_when_display_changes", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetHeight(root, 100);

    NodeRef child0 = NodeCreate();
    NodeStyleSetFlexGrow(child0, 1);
    NodeRef child1 = NodeCreate();
    NodeStyleSetFlexGrow(child1, 1);

    NodeRef child1_child0 = NodeCreate();
    NodeRef child1_child0_child0 = NodeCreate();
    NodeStyleSetWidth(child1_child0_child0, 8);
    NodeStyleSetHeight(child1_child0_child0, 16);

    NodeInsertChild(child1_child0, child1_child0_child0, 0);

    NodeInsertChild(child1, child1_child0, 0);
    NodeInsertChild(root, child0, 0);
    NodeInsertChild(root, child1, 0);

    NodeStyleSetDisplay(child0, DisplayType::Flex);
    NodeStyleSetDisplay(child1, DisplayType::None);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(0 == NodeLayoutGetWidth(child1_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(child1_child0_child0));

    NodeStyleSetDisplay(child0, DisplayType::None);
    NodeStyleSetDisplay(child1, DisplayType::Flex);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(8 == NodeLayoutGetWidth(child1_child0_child0));
    REQUIRE(16 == NodeLayoutGetHeight(child1_child0_child0));

    NodeStyleSetDisplay(child0, DisplayType::Flex);
    NodeStyleSetDisplay(child1, DisplayType::None);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(0 == NodeLayoutGetWidth(child1_child0_child0));
    REQUIRE(0 == NodeLayoutGetHeight(child1_child0_child0));

    NodeStyleSetDisplay(child0, DisplayType::None);
    NodeStyleSetDisplay(child1, DisplayType::Flex);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(8 == NodeLayoutGetWidth(child1_child0_child0));
    REQUIRE(16 == NodeLayoutGetHeight(child1_child0_child0));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_node_only_if_children_are_actually_removed", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef child0 = NodeCreate();
    NodeStyleSetWidth(child0, 50);
    NodeStyleSetHeight(child0, 25);
    NodeInsertChild(root, child0, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeRef child1 = NodeCreate();
    NodeRemoveChild(root, child1);
    REQUIRE(!NodeIsDirty(root));
    NodeDestroy(child1);

    NodeRemoveChild(root, child0);
    REQUIRE(NodeIsDirty(root));
    NodeDestroy(child0);

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::dirty_node_only_if_undefined_values_gets_set_to_undefined", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);
    NodeStyleSetMinWidth(root, Undefined);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(!NodeIsDirty(root));

    NodeStyleSetMinWidth(root, Undefined);

    REQUIRE(!NodeIsDirty(root));

    NodeDestroyRecursive(root);
  }
}