#include "Krystal.Core/Geometry/Polygon.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  TEST_CASE("PolygonF: vertices, edges and bounds", "[Geometry][PolygonF]")
  {
    PolygonF triangle(List<PointF> {{0.f, 0.f}, {10.f, 0.f}, {0.f, 20.f}});
    REQUIRE(triangle.NumberOfVertices() == 3u);
    REQUIRE(triangle.NumberOfEdges() == 3u);
    CHECK_FALSE(triangle.IsEmpty());
    CHECK(triangle.BoundingBox() == RectF(0.f, 0.f, 10.f, 20.f));

    const auto &edge = triangle.EdgeAt(0u);
    CHECK(edge.Vertex1() == PointF(0.f, 0.f));
    CHECK(edge.Vertex2() == PointF(10.f, 0.f));
    CHECK(&edge.NextEdge() == &triangle.EdgeAt(1u));
    CHECK(&edge.PreviousEdge() == &triangle.EdgeAt(2u));
    CHECK(edge.Polygon() == &triangle);
  }

  TEST_CASE("PolygonF: overlapping edges and intersections", "[Geometry][PolygonF]")
  {
    PolygonF square(List<PointF> {{0.f, 0.f}, {10.f, 0.f}, {10.f, 10.f}, {0.f, 10.f}});
    // The vertical edges span y 0 to 10; the horizontal ones lie on y 0 and y 10.
    CHECK(square.OverlappingEdges(4.f, 6.f).size() == 2u);

    PointF point;
    CHECK(square.EdgeAt(0u).Intersection(square.EdgeAt(1u), point));
    CHECK(point == PointF(10.f, 0.f));
    CHECK_FALSE(square.EdgeAt(0u).Intersection(square.EdgeAt(2u), point));
  }
}
