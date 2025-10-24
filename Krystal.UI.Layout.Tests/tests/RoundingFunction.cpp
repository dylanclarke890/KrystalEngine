#include "Krystal.UI.Layout/Algorithm/PixelGrid.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>
#include <cmath>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  TEST_CASE("LayoutEngine::rounding_value", "[LayoutEngine]")
  {
    // Test that whole numbers are rounded to whole despite ceil/floor flags
    REQUIRE(6.0 == RoundValueToPixelGrid(6.000001, 2.0, false, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(6.000001, 2.0, true, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(6.000001, 2.0, false, true));
    REQUIRE(6.0 == RoundValueToPixelGrid(5.999999, 2.0, false, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(5.999999, 2.0, true, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(5.999999, 2.0, false, true));
    // Same tests for negative numbers
    REQUIRE(-6.0 == RoundValueToPixelGrid(-6.000001, 2.0, false, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-6.000001, 2.0, true, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-6.000001, 2.0, false, true));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-5.999999, 2.0, false, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-5.999999, 2.0, true, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-5.999999, 2.0, false, true));

    // Test that numbers with fraction are rounded correctly accounting for
    // ceil/floor flags
    REQUIRE(6.0 == RoundValueToPixelGrid(6.01, 2.0, false, false));
    REQUIRE(6.5 == RoundValueToPixelGrid(6.01, 2.0, true, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(6.01, 2.0, false, true));
    REQUIRE(6.0 == RoundValueToPixelGrid(5.99, 2.0, false, false));
    REQUIRE(6.0 == RoundValueToPixelGrid(5.99, 2.0, true, false));
    REQUIRE(5.5 == RoundValueToPixelGrid(5.99, 2.0, false, true));
    // Same tests for negative numbers
    REQUIRE(-6.0 == RoundValueToPixelGrid(-6.01, 2.0, false, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-6.01, 2.0, true, false));
    REQUIRE(-6.5 == RoundValueToPixelGrid(-6.01, 2.0, false, true));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-5.99, 2.0, false, false));
    REQUIRE(-5.5 == RoundValueToPixelGrid(-5.99, 2.0, true, false));
    REQUIRE(-6.0 == RoundValueToPixelGrid(-5.99, 2.0, false, true));

    // Rounding up/down halfway values is as expected for both positive and
    // negative numbers
    REQUIRE(-3 == RoundValueToPixelGrid(-3.5, 1.0, false, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.4, 1.0, false, false));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.6, 1.0, false, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.499999, 1.0, false, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.500001, 1.0, false, false));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.5001, 1.0, false, false));

    REQUIRE(-3 == RoundValueToPixelGrid(-3.5, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.4, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.6, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.499999, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.500001, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.5001, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.00001, 1.0, true, false));
    REQUIRE(-3 == RoundValueToPixelGrid(-3, 1.0, true, false));

    REQUIRE(-4 == RoundValueToPixelGrid(-3.5, 1.0, false, true));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.4, 1.0, false, true));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.6, 1.0, false, true));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.499999, 1.0, false, true));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.500001, 1.0, false, true));
    REQUIRE(-4 == RoundValueToPixelGrid(-3.5001, 1.0, false, true));
    REQUIRE(-3 == RoundValueToPixelGrid(-3.00001, 1.0, false, true));
    REQUIRE(-3 == RoundValueToPixelGrid(-3, 1.0, false, true));

    // NAN is treated as expected:
    REQUIRE(std::isnan(RoundValueToPixelGrid(std::numeric_limits<double>::quiet_NaN(), 1.5, false, false)));
    REQUIRE(std::isnan(RoundValueToPixelGrid(1.5, std::numeric_limits<double>::quiet_NaN(), false, false)));
    REQUIRE(std::isnan(RoundValueToPixelGrid(std::numeric_limits<double>::quiet_NaN(),
                                             std::numeric_limits<double>::quiet_NaN(), false, false)));
  }

  static Size measureText(NodeConstRef /*node*/, float /*width*/, MeasureMode /*widthMode*/, float /*height*/,
                          MeasureMode /*heightMode*/)
  {
    return Size {10, 10};
  }

  // Regression test for https://github.com/facebook/yoga/issues/824
  TEST_CASE("LayoutEngine::consistent_rounding_during_repeated_layouts", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetPointScaleFactor(config, 2);

    NodeRef root = NodeCreate(config);
    NodeStyleSetMargin(root, Edge::Top, -1.49f);
    NodeStyleSetWidth(root, 500);
    NodeStyleSetHeight(root, 500);

    NodeRef node0 = NodeCreate(config);
    NodeInsertChild(root, node0, 0);

    NodeRef node1 = NodeCreate(config);
    NodeSetMeasureFunc(node1, measureText);
    NodeInsertChild(node0, node1, 0);

    for (int i = 0; i < 5; i++)
    {
      // Dirty the tree so YGRoundToPixelGrid runs again
      NodeStyleSetMargin(root, Edge::Left, (float)(i + 1));

      NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);
      REQUIRE(10 == NodeLayoutGetHeight(node1));
    }

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  // Regression test for https://github.com/facebook/yoga/issues/1860
  TEST_CASE("LayoutEngine::roundLayoutResultsToPixelGrid_height_rounding_up", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetPointScaleFactor(config, 3);

    NodeRef node = NodeCreate(config);
    NodeStyleSetPositionType(node, PositionType::Absolute);

    // These are values extracted from a debugging session in a real iOS app
    NodeStyleSetPosition(node, Edge::Left, 38.333333969116211f);
    NodeStyleSetPosition(node, Edge::Top, 1970.3333333432674f);
    NodeStyleSetWidth(node, 339.66665649414063f);
    NodeStyleSetHeight(node, 96);
    NodeSetNodeType(node, NodeType::Text);

    NodeCalculateLayout(node, Undefined, Undefined, Direction::LTR);

    // If this value is anything less than 96, iOS will not wrap the text to a 4th line
    REQUIRE(NodeLayoutGetHeight(node) == 96.0f);

    NodeDestroyRecursive(node);
    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::per_node_point_scale_factor", "[LayoutEngine]")
  {
    ConfigRef config1 = ConfigCreate();
    ConfigSetPointScaleFactor(config1, 2);

    ConfigRef config2 = ConfigCreate();
    ConfigSetPointScaleFactor(config2, 1);

    ConfigRef config3 = ConfigCreate();
    ConfigSetPointScaleFactor(config3, 0.5f);

    NodeRef root = NodeCreate(config1);
    NodeStyleSetWidth(root, 11.5);
    NodeStyleSetHeight(root, 11.5);

    NodeRef node0 = NodeCreate(config2);
    NodeStyleSetWidth(node0, 9.5);
    NodeStyleSetHeight(node0, 9.5);
    NodeInsertChild(root, node0, 0);

    NodeRef node1 = NodeCreate(config3);
    NodeStyleSetWidth(node1, 7);
    NodeStyleSetHeight(node1, 7);
    NodeInsertChild(node0, node1, 0);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(NodeLayoutGetWidth(root) == 11.5);
    REQUIRE(NodeLayoutGetHeight(root) == 11.5);

    REQUIRE(NodeLayoutGetWidth(node0) == 10);
    REQUIRE(NodeLayoutGetHeight(node0) == 10);

    REQUIRE(NodeLayoutGetWidth(node1) == 8);
    REQUIRE(NodeLayoutGetHeight(node1) == 8);

    NodeDestroyRecursive(root);

    ConfigDestroy(config1);
    ConfigDestroy(config2);
    ConfigDestroy(config3);
  }

  TEST_CASE("LayoutEngine::raw_layout_dimensions", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();
    ConfigSetPointScaleFactor(config, 0.5f);

    NodeRef root = NodeCreate(config);
    NodeStyleSetWidth(root, 11.5f);
    NodeStyleSetHeight(root, 9.5f);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(NodeLayoutGetWidth(root) == 12.0f);
    REQUIRE(NodeLayoutGetHeight(root) == 10.0f);
    REQUIRE(NodeLayoutGetRawWidth(root) == 11.5f);
    REQUIRE(NodeLayoutGetRawHeight(root) == 9.5f);

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}