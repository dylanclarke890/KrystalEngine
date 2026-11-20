/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Core/Geometry/PointGraph.hpp"
#include "Krystal.Core/Geometry/GeometryUtilities.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys
{
  PointGraphF::Node *PointGraphF::FindOrCreateNode(PointF point)
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

    _allNodes.push_back(CreateUnique<PointGraphF::Node>(point));
    return _allNodes.back().get();
  }

  void PointGraphF::Reset() noexcept
  {
    for (auto &node : _allNodes)
    {
      node->Reset();
    }
  }

  KRYS_NODISCARD static bool FindLineSegmentIntersection(const PointGraphF::Edge &edgeA,
                                                         const PointGraphF::Edge &edgeB,
                                                         PointF &intersectionPoint) noexcept
  {
    if (!FindIntersection(*edgeA.first, *edgeA.second, *edgeB.first, *edgeB.second, intersectionPoint))
    {
      return false;
    }

    PointF edgeAVec = ToPointF(*edgeA.second - *edgeA.first);
    PointF edgeBVec = ToPointF(*edgeB.second - *edgeB.first);

    float dotA = edgeAVec.Dot(ToPointF(intersectionPoint - *edgeA.first));
    if (dotA < 0 || dotA > edgeAVec.LengthSquared())
    {
      return false;
    }

    float dotB = edgeBVec.Dot(ToPointF(intersectionPoint - *edgeB.first));
    if (dotB < 0 || dotB > edgeBVec.LengthSquared())
    {
      return false;
    }

    return true;
  }

  static bool AddIntersectionPoints(List<PointGraphF::Polygon> &polys, PointGraphF &graph)
  {
    bool foundAnyIntersections = false;

    List<PointGraphF::Edge> allEdges;
    for (auto &poly : polys)
    {
      allEdges.insert(allEdges.end(), poly.begin(), poly.end());
    }

    for (const PointGraphF::Edge &edgeA : allEdges)
    {
      List<PointGraphF::Node *> intersectionPoints({edgeA.first, edgeA.second});

      for (const PointGraphF::Edge &edgeB : allEdges)
      {
        if (&edgeA == &edgeB)
        {
          continue;
        }

        PointF intersectionPoint;
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
                          return ToPointF(*edgeA.first - *b).LengthSquared()
                                 > ToPointF(*edgeA.first - *a).LengthSquared();
                        });

      for (size_t pointIndex = 1; pointIndex < intersectionPoints.size(); pointIndex++)
      {
        intersectionPoints[pointIndex - 1]->AddNextPoint(intersectionPoints[pointIndex]);
      }
    }

    return foundAnyIntersections;
  }

  KRYS_NODISCARD static PointGraphF::Polygon WalkGraphAndExtractPolygon(PointGraphF::Node *startNode)
  {
    PointGraphF::Polygon outPoly;

    PointGraphF::Node *currentNode = startNode;
    PointGraphF::Node *previousNode = startNode;

    do
    {
      currentNode->Visit();

      PointF currentVec = ToPointF(*previousNode - *currentNode);
      currentVec.Normalise();

      // Walk the graph, at each node choosing the next non-visited
      // point with the greatest internal angle.
      PointGraphF::Node *nextNode = nullptr;
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

        PointF nextVec = ToPointF(*potentialNextNode - *currentNode);
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
        return PointGraphF::Polygon();
      }

      outPoly.emplace_back(currentNode, nextNode);

      previousNode = currentNode;
      currentNode = nextNode;
    } while (currentNode != startNode);

    return outPoly;
  }

  KRYS_NODISCARD static PointGraphF::Node *FindUnvisitedPolygonStartPoint(List<PointGraphF::Polygon> &polys)
  {
    for (auto &poly : polys)
    {
      bool visited = std::ranges::any_of(poly, [](const PointGraphF::Edge &edge)
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

  KRYS_NODISCARD static List<PointGraphF::Polygon> UnitePolygons(List<PointGraphF::Polygon> &polys,
                                                                 PointGraphF &graph)
  {
    graph.Reset();

    // There are no intersections, so the polygons are disjoint (we already removed wholly-contained rects in
    // an earlier step).
    if (!AddIntersectionPoints(polys, graph))
    {
      return polys;
    }

    List<PointGraphF::Polygon> unitedPolygons;

    while (PointGraphF::Node *startNode = FindUnvisitedPolygonStartPoint(polys))
    {
      PointGraphF::Polygon unitedPolygon = WalkGraphAndExtractPolygon(startNode);
      if (unitedPolygon.empty())
      {
        return List<PointGraphF::Polygon>();
      }
      unitedPolygons.push_back(unitedPolygon);
    }

    return unitedPolygons;
  }

  KRYS_NODISCARD static PointGraphF::Polygon EdgesForRect(RectF rect, PointGraphF &graph)
  {
    auto minMin = graph.FindOrCreateNode(rect.MinXMinYCorner());
    auto minMax = graph.FindOrCreateNode(rect.MinXMaxYCorner());
    auto maxMax = graph.FindOrCreateNode(rect.MaxXMaxYCorner());
    auto maxMin = graph.FindOrCreateNode(rect.MaxXMinYCorner());

    return PointGraphF::Polygon({std::make_pair(minMin, maxMin), std::make_pair(maxMin, maxMax),
                                 std::make_pair(maxMax, minMax), std::make_pair(minMax, minMin)});
  }

  std::pair<PointGraphF, List<PointGraphF::Polygon>> PointGraphF::PolygonsForRect(const List<RectF> &rects)
  {
    List<RectF> sortedRects = rects;
    // FIXME: Replace it with 2 dimensional sort.
    std::ranges::stable_sort(sortedRects, {}, &RectF::X);
    std::ranges::stable_sort(sortedRects, {}, &RectF::Y);

    PointGraphF graph;

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
