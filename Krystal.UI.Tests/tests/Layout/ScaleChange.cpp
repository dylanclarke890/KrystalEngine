#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::scale_change_invalidates_layout", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    ConfigSetPointScaleFactor(config, 1.0f);

    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child0, 1);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetLeft(root_child1));

    ConfigSetPointScaleFactor(config, 1.5f);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    // Left should change due to pixel alignment of new scale factor
    REQUIRE(25.333334f == NodeLayoutGetLeft(root_child1));

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::errata_config_change_relayout", "[LayoutEngine]")
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

    ConfigSetErrata(config, Errata::None);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    // This should be modified by the lack of the errata
    REQUIRE(0 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0));
    // This should be modified by the lack of the errata
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0));
    // This should be modified by the lack of the errata
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::setting_compatible_config_maintains_layout_cache", "[LayoutEngine]")
  {
    static uint32_t measureCallCount = 0;
    auto measureCustom = [](NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/,
                            float /*height*/, MeasureMode /*heightMode*/) -> Size
    {
      measureCallCount++;
      return Size {25.0f, 25.0f};
    };

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    ConfigSetPointScaleFactor(config, 1.0f);

    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetWidth(root, 50);
    NodeStyleSetHeight(root, 50);

    NodeRef root_child0 = NodeCreate(config);
    REQUIRE(0 == measureCallCount);

    NodeSetMeasureFunc(root_child0, measureCustom);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetFlexGrow(root_child1, 1);
    NodeInsertChild(root, root_child1, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
    REQUIRE(1 == measureCallCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetLeft(root_child1));

    ConfigRef config2 = ConfigCreate();
    // Calling ConfigSetPointScaleFactor multiple times, ensures that config2
    // gets a different config version that config1
    ConfigSetPointScaleFactor(config2, 1.0f);
    ConfigSetPointScaleFactor(config2, 1.5f);
    ConfigSetPointScaleFactor(config2, 1.0f);

    NodeSetConfig(root, config2);
    NodeSetConfig(root_child0, config2);
    NodeSetConfig(root_child1, config2);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    // Measure should not be called again, as layout should have been cached since
    // config is functionally the same as before
    REQUIRE(1 == measureCallCount);
    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(25 == NodeLayoutGetLeft(root_child1));

    NodeDestroyRecursive(root);
    ConfigDestroy(config);
    ConfigDestroy(config2);
  }
}