#pragma once

#include "Krystal.HTML/DOM/BoundaryPoint.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#dictdef-staticrangeinit
  class StaticRangeInit
  {
    BoundaryPoint _start;
    BoundaryPoint _end;

    StaticRangeInit(BoundaryPoint start, BoundaryPoint end) noexcept
        : _start {Krys::Move(start)}, _end {Krys::Move(end)}
    {
    }

  public:
    static ExceptionOr<StaticRangeInit> Create(BoundaryPoint start, BoundaryPoint end) noexcept;

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