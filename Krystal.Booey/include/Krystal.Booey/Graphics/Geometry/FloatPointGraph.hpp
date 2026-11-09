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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  /// @brief A graph of points joined by edges, for uniting rects into the polygons that outline them.
  class FloatPointGraph
  {
    KRYS_NON_COPYABLE(FloatPointGraph);

  public:
    /// @brief A point and the points its edges lead to.
    class Node : public FloatPoint
    {
      KRYS_NON_COPYABLE(Node);

    private:
      List<Node *> _nextPoints;
      bool _visited {false};

    public:
      explicit Node(FloatPoint point) noexcept : FloatPoint(point)
      {
      }

      KRYS_NODISCARD const List<Node *> &NextPoints() const noexcept
      {
        return _nextPoints;
      }

      void AddNextPoint(Node *node) noexcept
      {
        if (std::ranges::find(_nextPoints, node) == _nextPoints.end())
        {
          _nextPoints.push_back(node);
        }
      }

      KRYS_NODISCARD bool IsVisited() const noexcept
      {
        return _visited;
      }
      void Visit() noexcept
      {
        _visited = true;
      }

      void Reset() noexcept
      {
        _visited = false;
        _nextPoints.clear();
      }
    };

    using Edge = std::pair<Node *, Node *>;
    using Polygon = List<Edge>;

  private:
    List<UniquePtr<Node>> _allNodes;

  public:
    FloatPointGraph() noexcept = default;
    FloatPointGraph(FloatPointGraph &&) noexcept = default;
    FloatPointGraph &operator=(FloatPointGraph &&) noexcept = default;

    /// @brief The polygons outlining the union of @p rects, or none when it has holes; the graph owns their
    /// points.
    KRYS_NODISCARD static std::pair<FloatPointGraph, List<Polygon>>
      PolygonsForRect(const List<FloatRect> &rects);

    /// @brief The node at @p point (within a small tolerance), made if there isn't one.
    KRYS_NODISCARD Node *FindOrCreateNode(FloatPoint point);
    void Reset() noexcept;
  };
}
