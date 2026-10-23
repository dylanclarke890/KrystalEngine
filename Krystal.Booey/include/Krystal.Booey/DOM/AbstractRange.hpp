#pragma once

#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#interface-abstractrange
  class AbstractRange : public RefCounted<AbstractRange>
  {
  protected:
    BoundaryPoint _start;
    BoundaryPoint _end;

  protected:
    AbstractRange(BoundaryPoint start, BoundaryPoint end) noexcept : _start(start), _end(end)
    {
    }

  public:
    virtual ~AbstractRange() noexcept = default;

    /// @brief Whether it's a live `Range`, which follows changes to the tree.
    KRYS_NODISCARD virtual bool IsLiveRange() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD const BoundaryPoint &StartBoundaryPoint() const noexcept
    {
      return _start;
    }

    KRYS_NODISCARD const BoundaryPoint &EndBoundaryPoint() const noexcept
    {
      return _end;
    }

#pragma region AbstractRange - https://dom.spec.whatwg.org/#abstractrange

    /// @see https://dom.spec.whatwg.org/#dom-range-startcontainer
    KRYS_NODISCARD Node *StartContainer() const noexcept
    {
      return _start.Container.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-startoffset
    KRYS_NODISCARD uint64 StartOffset() const noexcept
    {
      return _start.Offset;
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-endcontainer
    KRYS_NODISCARD Node *EndContainer() const noexcept
    {
      return _end.Container.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-endoffset
    KRYS_NODISCARD uint64 EndOffset() const noexcept
    {
      return _end.Offset;
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-collapsed
    KRYS_NODISCARD bool Collapsed() const noexcept
    {
      return _start.Container == _end.Container && _start.Offset == _end.Offset;
    }

#pragma endregion
  };

  /// @brief The boundary points of @p range, as they are now.
  KRYS_NODISCARD inline SimpleRange MakeSimpleRange(const AbstractRange &range) noexcept
  {
    return {range.StartBoundaryPoint(), range.EndBoundaryPoint()};
  }
}