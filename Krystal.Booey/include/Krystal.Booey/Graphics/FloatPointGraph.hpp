#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
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
