#pragma once

#include "Krystal.HTML/DOM/BoundaryPoint.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
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

#pragma region AbstractRange - https://dom.spec.whatwg.org/#abstractrange

    /// @see https://dom.spec.whatwg.org/#dom-range-startcontainer
    KRYS_NODISCARD RawPtr<Node> StartContainer() const noexcept
    {
      return _start.Container.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-startoffset
    KRYS_NODISCARD uint64 StartOffset() const noexcept
    {
      return _start.Offset;
    }

    /// @see https://dom.spec.whatwg.org/#dom-range-endcontainer
    KRYS_NODISCARD RawPtr<Node> EndContainer() const noexcept
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
}