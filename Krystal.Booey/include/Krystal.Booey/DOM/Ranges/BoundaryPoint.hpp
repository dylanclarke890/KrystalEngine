#pragma once

#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <compare>

namespace krys::boo::dom
{
  class Node;

  /// @brief A node and an offset into it.
  /// @see https://dom.spec.whatwg.org/#boundary-points
  /// @see https://dom.spec.whatwg.org/#concept-range-bp
  struct BoundaryPoint
  {
    /// @see https://dom.spec.whatwg.org/#boundary-point-node
    Ref<Node> Container;
    /// @see https://dom.spec.whatwg.org/#concept-range-bp-offset
    uint64 Offset;

    /// @brief The position of this boundary point relative to @p other, which must have the same root: less
    /// is before, equal is equal, and greater is after.
    /// @see https://dom.spec.whatwg.org/#concept-range-bp-position
    /// @see https://dom.spec.whatwg.org/#concept-range-bp-before
    /// @see https://dom.spec.whatwg.org/#concept-range-bp-equal
    /// @see https://dom.spec.whatwg.org/#concept-range-bp-after
    KRYS_NODISCARD std::strong_ordering ComparePositionTo(const BoundaryPoint &other) const noexcept;

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
