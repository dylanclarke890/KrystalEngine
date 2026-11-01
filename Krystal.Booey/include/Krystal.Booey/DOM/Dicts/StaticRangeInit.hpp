#pragma once

#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-staticrangeinit
  class StaticRangeInit
  {
    BoundaryPoint _start;
    BoundaryPoint _end;

    StaticRangeInit(BoundaryPoint start, BoundaryPoint end) noexcept
        : _start {krys::move(start)}, _end {krys::move(end)}
    {
    }

  public:
    /// @brief The start and end of a static range; throws an "InvalidNodeTypeError" if either is in a
    /// DocumentType or Attr node.
    /// @see https://dom.spec.whatwg.org/#dom-staticrange-staticrange
    KRYS_NODISCARD static ExceptionOr<StaticRangeInit> Create(BoundaryPoint start,
                                                              BoundaryPoint end) noexcept;

    KRYS_NODISCARD const BoundaryPoint &Start() const noexcept
    {
      return _start;
    }

    KRYS_NODISCARD const BoundaryPoint &End() const noexcept
    {
      return _end;
    }
  };
}