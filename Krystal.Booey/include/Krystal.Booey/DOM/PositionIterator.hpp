#pragma once

#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A Position iterator with constant-time increment, decrement, and several predicates on the
  /// Position it is at. Conversion to/from Position is O(n) in the offset.
  class PositionIterator
  {
  private:
    RefPtr<Node> _anchorNode;
    // If this is non-null, _nodeAfterPositionInAnchor->ParentNode() == _anchorNode;
    RefPtr<Node> _nodeAfterPositionInAnchor;
    int32 _offsetInAnchor {0};

  public:
    PositionIterator(const Position &position) noexcept;
    operator Position() const noexcept;

    void Increment() noexcept;
    void Decrement() noexcept;

    KRYS_NODISCARD Node *GetNode() const noexcept
    {
      return _anchorNode.get();
    }
    KRYS_NODISCARD int32 OffsetInLeafNode() const noexcept
    {
      return _offsetInAnchor;
    }

    KRYS_NODISCARD bool AtStart() const noexcept;
    KRYS_NODISCARD bool AtEnd() const noexcept;
    KRYS_NODISCARD bool AtStartOfNode() const noexcept;
    KRYS_NODISCARD bool AtEndOfNode() const noexcept;
    KRYS_NODISCARD bool IsCandidate() const noexcept;
  };
}
