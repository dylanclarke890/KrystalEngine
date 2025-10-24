#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;
  static Size _measureFloor(NodeConstRef /*node*/, float width, MeasureMode /*widthMode*/, float height,
                            MeasureMode /*heightMode*/)
  {
    return Size {
      width = 10.2f,
      height = 10.2f,
    };
  }

  static Size _measureCeil(NodeConstRef /*node*/, float width, MeasureMode /*widthMode*/, float height,
                           MeasureMode /*heightMode*/)
  {
    return Size {
      width = 10.5f,
      height = 10.5f,
    };
  }

  static Size _measureFractial(NodeConstRef /*node*/, float width, MeasureMode /*widthMode*/, float height,
                               MeasureMode /*heightMode*/)
  {
    return Size {
      width = 0.5f,
      height = 0.5f,
    };
  }

  TEST_CASE("LayoutEngine::rounding_feature_with_custom_measure_func_floor", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    NodeRef root = NodeCreate(config);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _measureFloor);
    NodeInsertChild(root, root_child0, 0);

    ConfigSetPointScaleFactor(config, 0.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(10.2f == NodeLayoutGetWidth(root_child0));
    REQUIRE(10.2f == NodeLayoutGetHeight(root_child0));

    ConfigSetPointScaleFactor(config, 1.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(11 == NodeLayoutGetWidth(root_child0));
    REQUIRE(11 == NodeLayoutGetHeight(root_child0));

    ConfigSetPointScaleFactor(config, 2.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(10.5 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10.5 == NodeLayoutGetHeight(root_child0));

    ConfigSetPointScaleFactor(config, 4.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(10.25 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10.25 == NodeLayoutGetHeight(root_child0));

    ConfigSetPointScaleFactor(config, 1.0f / 3.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(12.0 == NodeLayoutGetWidth(root_child0));
    REQUIRE(12.0 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::rounding_feature_with_custom_measure_func_ceil", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    NodeRef root = NodeCreate(config);

    NodeRef root_child0 = NodeCreate(config);
    NodeSetMeasureFunc(root_child0, _measureCeil);
    NodeInsertChild(root, root_child0, 0);

    ConfigSetPointScaleFactor(config, 1.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(11 == NodeLayoutGetWidth(root_child0));
    REQUIRE(11 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::rounding_feature_with_custom_measure_and_fractial_matching_scale",
            "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetPosition(root_child0, Edge::Left, 73.625);
    NodeStyleSetPositionType(root_child0, PositionType::Relative);
    NodeSetMeasureFunc(root_child0, _measureFractial);
    NodeInsertChild(root, root_child0, 0);

    ConfigSetPointScaleFactor(config, 2.0f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0.5 == NodeLayoutGetWidth(root_child0));
    REQUIRE(0.5 == NodeLayoutGetHeight(root_child0));
    REQUIRE(73.5 == NodeLayoutGetLeft(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}