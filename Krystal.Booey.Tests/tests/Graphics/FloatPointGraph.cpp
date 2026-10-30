#include "Krystal.Booey/Graphics/FloatPointGraph.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FloatPointGraph: unites rects into the polygons outlining them", "[Graphics][FloatPointGraph]")
  {
    SECTION("one rect")
    {
      auto [graph, polygons] = FloatPointGraph::PolygonsForRect({FloatRect(0, 0, 10, 10)});
      REQUIRE(polygons.size() == 1u);
      CHECK(polygons[0].size() == 4u);
    }

    SECTION("overlapping rects make one polygon round both")
    {
      // Two lines of an inline box, the second shorter and overlapping the first.
      auto [graph, polygons] =
        FloatPointGraph::PolygonsForRect({FloatRect(0, 0, 30, 10), FloatRect(0, 8, 20, 10)});
      REQUIRE(polygons.size() == 1u);
      // Six corners, and the two points where one rect's corner lies on the other's left edge.
      CHECK(polygons[0].size() == 8u);
      for (auto &edge : polygons[0])
      {
        // The edges are axis-aligned and join end to start.
        CHECK((edge.first->X() == edge.second->X() || edge.first->Y() == edge.second->Y()));
      }
      for (size_t i = 0; i < polygons[0].size(); ++i)
      {
        CHECK(polygons[0][i].second == polygons[0][(i + 1) % polygons[0].size()].first);
      }
    }

    SECTION("disjoint rects stay apart")
    {
      auto [graph, polygons] =
        FloatPointGraph::PolygonsForRect({FloatRect(0, 0, 10, 10), FloatRect(20, 20, 10, 10)});
      CHECK(polygons.size() == 2u);
    }

    SECTION("a rect inside another is dropped")
    {
      auto [graph, polygons] =
        FloatPointGraph::PolygonsForRect({FloatRect(0, 0, 30, 30), FloatRect(10, 10, 5, 5)});
      REQUIRE(polygons.size() == 1u);
      CHECK(polygons[0].size() == 4u);
    }
  }
}
