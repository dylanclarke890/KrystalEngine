#include "Krystal.Booey/Graphics/FloatPointGraph.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo
{
  FloatPointGraph::Node *FloatPointGraph::FindOrCreateNode(FloatPoint point)
  {
    for (auto &testNode : _allNodes)
    {
      auto tolerance = [&](float a, float b)
      {
        auto toleranceInPixel = 0.005f;
        return toleranceInPixel / std::max(std::abs(a), std::abs(b)) * 100.f;
      };
      if (AreEssentiallyEqual(testNode->X(), point.X(), tolerance(testNode->X(), point.X()))
          && AreEssentiallyEqual(testNode->Y(), point.Y(), tolerance(testNode->Y(), point.Y())))
      {
        return testNode.get();
      }
    }

    _allNodes.push_back(CreateUnique<FloatPointGraph::Node>(point));
    return _allNodes.back().get();
  }

  void FloatPointGraph::Reset() noexcept
  {
    for (auto &node : _allNodes)
    {
      node->Reset();
    }
  }

  KRYS_NODISCARD static bool FindLineSegmentIntersection(const FloatPointGraph::Edge &edgeA,
                                                         const FloatPointGraph::Edge &edgeB,
                                                         FloatPoint &intersectionPoint) noexcept
  {
    if (!FindIntersection(*edgeA.first, *edgeA.second, *edgeB.first, *edgeB.second, intersectionPoint))
    {
      return false;
    }

    FloatPoint edgeAVec = ToFloatPoint(*edgeA.second - *edgeA.first);
    FloatPoint edgeBVec = ToFloatPoint(*edgeB.second - *edgeB.first);

    float dotA = edgeAVec.Dot(ToFloatPoint(intersectionPoint - *edgeA.first));
    if (dotA < 0 || dotA > edgeAVec.LengthSquared())
    {
      return false;
    }

    float dotB = edgeBVec.Dot(ToFloatPoint(intersectionPoint - *edgeB.first));
    if (dotB < 0 || dotB > edgeBVec.LengthSquared())
    {
      return false;
    }

    return true;
  }

  static bool AddIntersectionPoints(List<FloatPointGraph::Polygon> &polys, FloatPointGraph &graph)
  {
    bool foundAnyIntersections = false;

    List<FloatPointGraph::Edge> allEdges;
    for (auto &poly : polys)
    {
      allEdges.insert(allEdges.end(), poly.begin(), poly.end());
    }

    for (const FloatPointGraph::Edge &edgeA : allEdges)
    {
      List<FloatPointGraph::Node *> intersectionPoints({edgeA.first, edgeA.second});

      for (const FloatPointGraph::Edge &edgeB : allEdges)
      {
        if (&edgeA == &edgeB)
        {
          continue;
        }

        FloatPoint intersectionPoint;
        if (!FindLineSegmentIntersection(edgeA, edgeB, intersectionPoint))
        {
          continue;
        }
        foundAnyIntersections = true;
        intersectionPoints.push_back(graph.FindOrCreateNode(intersectionPoint));
      }

      std::ranges::sort(intersectionPoints,
                        [edgeA](auto *a, auto *b)
                        {
                          return ToFloatPoint(*edgeA.first - *b).LengthSquared()
                                 > ToFloatPoint(*edgeA.first - *a).LengthSquared();
                        });

      for (size_t pointIndex = 1; pointIndex < intersectionPoints.size(); pointIndex++)
      {
        intersectionPoints[pointIndex - 1]->AddNextPoint(intersectionPoints[pointIndex]);
      }
    }

    return foundAnyIntersections;
  }

  KRYS_NODISCARD static FloatPointGraph::Polygon WalkGraphAndExtractPolygon(FloatPointGraph::Node *startNode)
  {
    FloatPointGraph::Polygon outPoly;

    FloatPointGraph::Node *currentNode = startNode;
    FloatPointGraph::Node *previousNode = startNode;

    do
    {
      currentNode->Visit();

      FloatPoint currentVec = ToFloatPoint(*previousNode - *currentNode);
      currentVec.Normalise();

      // Walk the graph, at each node choosing the next non-visited
      // point with the greatest internal angle.
      FloatPointGraph::Node *nextNode = nullptr;
      float nextNodeAngle = 0;
      for (auto *potentialNextNode : currentNode->NextPoints())
      {
        if (potentialNextNode == currentNode)
        {
          continue;
        }

        // If we can get back to the start, we should, ignoring the fact that we already visited it.
        // Otherwise we'll head inside the shape.
        if (potentialNextNode == startNode)
        {
          nextNode = startNode;
          break;
        }

        if (potentialNextNode->IsVisited())
        {
          continue;
        }

        FloatPoint nextVec = ToFloatPoint(*potentialNextNode - *currentNode);
        nextVec.Normalise();

        float angle = std::acos(nextVec.Dot(currentVec));
        float crossZ = nextVec.X() * currentVec.Y() - nextVec.Y() * currentVec.X();

        if (crossZ < 0)
        {
          angle = (2 * std::numbers::pi_v<float>)-angle;
        }

        if (!nextNode || angle > nextNodeAngle)
        {
          nextNode = potentialNextNode;
          nextNodeAngle = angle;
        }
      }

      // If we don't end up at a node adjacent to the starting node,
      // something went wrong (there's probably a hole in the shape),
      // so bail out. We'll use a bounding box instead.
      if (!nextNode)
      {
        return FloatPointGraph::Polygon();
      }

      outPoly.push_back(std::make_pair(currentNode, nextNode));

      previousNode = currentNode;
      currentNode = nextNode;
    } while (currentNode != startNode);

    return outPoly;
  }

  KRYS_NODISCARD static FloatPointGraph::Node *
    FindUnvisitedPolygonStartPoint(List<FloatPointGraph::Polygon> &polys)
  {
    for (auto &poly : polys)
    {
      bool visited = std::ranges::any_of(poly, [](const FloatPointGraph::Edge &edge)
                                         { return edge.first->IsVisited() || edge.second->IsVisited(); });
      if (visited)
      {
        continue;
      }

      // FIXME: We should make sure we find an outside edge to start with.
      return poly[0].first;
    }
    return nullptr;
  }

  KRYS_NODISCARD static List<FloatPointGraph::Polygon> UnitePolygons(List<FloatPointGraph::Polygon> &polys,
                                                                     FloatPointGraph &graph)
  {
    graph.Reset();

    // There are no intersections, so the polygons are disjoint (we already removed wholly-contained rects in
    // an earlier step).
    if (!AddIntersectionPoints(polys, graph))
    {
      return polys;
    }

    List<FloatPointGraph::Polygon> unitedPolygons;

    while (FloatPointGraph::Node *startNode = FindUnvisitedPolygonStartPoint(polys))
    {
      FloatPointGraph::Polygon unitedPolygon = WalkGraphAndExtractPolygon(startNode);
      if (unitedPolygon.empty())
      {
        return List<FloatPointGraph::Polygon>();
      }
      unitedPolygons.push_back(unitedPolygon);
    }

    return unitedPolygons;
  }

  KRYS_NODISCARD static FloatPointGraph::Polygon EdgesForRect(FloatRect rect, FloatPointGraph &graph)
  {
    auto minMin = graph.FindOrCreateNode(rect.MinXMinYCorner());
    auto minMax = graph.FindOrCreateNode(rect.MinXMaxYCorner());
    auto maxMax = graph.FindOrCreateNode(rect.MaxXMaxYCorner());
    auto maxMin = graph.FindOrCreateNode(rect.MaxXMinYCorner());

    return FloatPointGraph::Polygon({std::make_pair(minMin, maxMin), std::make_pair(maxMin, maxMax),
                                     std::make_pair(maxMax, minMax), std::make_pair(minMax, minMin)});
  }

  std::pair<FloatPointGraph, List<FloatPointGraph::Polygon>>
    FloatPointGraph::PolygonsForRect(const List<FloatRect> &rects)
  {
    List<FloatRect> sortedRects = rects;
    // FIXME: Replace it with 2 dimensional sort.
    std::ranges::stable_sort(sortedRects, {}, &FloatRect::X);
    std::ranges::stable_sort(sortedRects, {}, &FloatRect::Y);

    FloatPointGraph graph;

    List<Polygon> rectPolygons;
    rectPolygons.reserve(sortedRects.size());

    for (auto &rect : sortedRects)
    {
      bool isContained = false;
      for (auto &otherRect : sortedRects)
      {
        if (&rect == &otherRect)
        {
          continue;
        }
        if (otherRect.Contains(rect))
        {
          isContained = true;
          break;
        }
      }

      if (!isContained)
      {
        rectPolygons.push_back(EdgesForRect(rect, graph));
      }
    }

    auto unitedPolygons = UnitePolygons(rectPolygons, graph);

    return {krys::move(graph), krys::move(unitedPolygons)};
  }
}
