#include "Krystal.Lib/Array.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::computed_layout_padding", "[LayoutEngine]")
  {
    NodeRef root = NodeCreate();
    NodeStyleSetWidth(root, 100);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetPaddingPercent(root, Edge::Start, 10);

    NodeCalculateLayout(root, 100, 100, Direction::LTR);

    REQUIRE(10 == NodeLayoutGetPadding(root, Edge::Left));
    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Right));

    NodeCalculateLayout(root, 100, 100, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetPadding(root, Edge::Left));
    REQUIRE(10 == NodeLayoutGetPadding(root, Edge::Right));

    NodeDestroyRecursive(root);
  }

  TEST_CASE("LayoutEngine::padding_side_overrides_horizontal_and_vertical", "[LayoutEngine]")
  {
    const Array<Edge, 6> edges = {{Edge::Top, Edge::Bottom, Edge::Start, Edge::End, Edge::Left, Edge::Right}};

    for (float edgeValue = 0; edgeValue < 2; ++edgeValue)
    {
      for (const auto &edge : edges)
      {
        Edge horizontalOrVertical =
          edge == Edge::Top || edge == Edge::Bottom ? Edge::Vertical : Edge::Horizontal;

        NodeRef root = NodeCreate();
        NodeStyleSetWidth(root, 100);
        NodeStyleSetHeight(root, 100);
        NodeStyleSetPadding(root, horizontalOrVertical, 10);
        NodeStyleSetPadding(root, edge, edgeValue);

        NodeCalculateLayout(root, 100, 100, Direction::LTR);

        REQUIRE(edgeValue == NodeLayoutGetPadding(root, edge));

        NodeDestroyRecursive(root);
      }
    }
  }

  TEST_CASE("LayoutEngine::padding_side_overrides_all", "[LayoutEngine]")
  {
    const Array<Edge, 6> edges = {{Edge::Top, Edge::Bottom, Edge::Start, Edge::End, Edge::Left, Edge::Right}};

    for (float edgeValue = 0; edgeValue < 2; ++edgeValue)
    {
      for (const auto &edge : edges)
      {
        NodeRef root = NodeCreate();
        NodeStyleSetWidth(root, 100);
        NodeStyleSetHeight(root, 100);
        NodeStyleSetPadding(root, Edge::All, 10);
        NodeStyleSetPadding(root, edge, edgeValue);

        NodeCalculateLayout(root, 100, 100, Direction::LTR);

        REQUIRE(edgeValue == NodeLayoutGetPadding(root, edge));

        NodeDestroyRecursive(root);
      }
    }
  }

  TEST_CASE("LayoutEngine::padding_horizontal_and_vertical_overrides_all", "[LayoutEngine]")
  {
    const Array<Edge, 2> directions = {{Edge::Horizontal, Edge::Vertical}};

    for (float directionValue = 0; directionValue < 2; ++directionValue)
    {
      for (const auto &direction : directions)
      {
        NodeRef root = NodeCreate();
        NodeStyleSetWidth(root, 100);
        NodeStyleSetHeight(root, 100);
        NodeStyleSetPadding(root, Edge::All, 10);
        NodeStyleSetPadding(root, direction, directionValue);

        NodeCalculateLayout(root, 100, 100, Direction::LTR);

        if (direction == Edge::Vertical)
        {
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::Top));
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::Bottom));
        }
        else
        {
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::Start));
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::End));
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::Left));
          REQUIRE(directionValue == NodeLayoutGetPadding(root, Edge::Right));
        }

        NodeDestroyRecursive(root);
      }
    }
  }
}