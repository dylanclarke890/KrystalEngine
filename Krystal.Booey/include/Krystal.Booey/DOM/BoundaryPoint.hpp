#pragma once

#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <compare>

namespace krys::boo::dom
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#boundary-points
  struct BoundaryPoint
  {
    Ref<Node> Container;
    uint64 Offset;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-position
    std::strong_ordering ComparePositionTo(const BoundaryPoint &other) const noexcept;

    KRYS_NODISCARD Document &GetDocument() const noexcept
    {
      return Container->NodeDocument();
    }

    KRYS_NODISCARD friend bool operator==(const BoundaryPoint &a, const BoundaryPoint &b) noexcept
    {
      return a.Container.get() == b.Container.get() && a.Offset == b.Offset;
    }
  };

  /// @brief The order of two boundary points in a tree, or unordered if they aren't in the same one.
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD std::partial_ordering TreeOrder(const BoundaryPoint &a, const BoundaryPoint &b) noexcept;

  /// @brief The boundary point before @p node in its parent, or null if it has no parent.
  KRYS_NODISCARD Maybe<BoundaryPoint> MakeBoundaryPointBeforeNode(Node &node) noexcept;
  /// @brief The boundary point after @p node in its parent, or null if it has no parent.
  KRYS_NODISCARD Maybe<BoundaryPoint> MakeBoundaryPointAfterNode(Node &node) noexcept;

  KRYS_NODISCARD inline BoundaryPoint MakeBoundaryPointBeforeNodeContents(Node &node) noexcept
  {
    return {ShareRef(node), 0};
  }

  KRYS_NODISCARD inline BoundaryPoint MakeBoundaryPointAfterNodeContents(Node &node) noexcept
  {
    return {ShareRef(node), node.Length()};
  }
}
