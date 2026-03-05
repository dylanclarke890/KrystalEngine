#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Ranges/BoundaryPoint.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys::HTML
{
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

    KRYS_NODISCARD bool IsCollapsed() const noexcept
    {
      return _start.Container == _end.Container && _start.Offset == _end.Offset;
    }

    KRYS_NODISCARD RawPtr<Node> StartContainer() const noexcept
    {
      return _start.Container.get();
    }

    KRYS_NODISCARD uint64 StartOffset() const noexcept
    {
      return _start.Offset;
    }

    KRYS_NODISCARD RawPtr<Node> EndContainer() const noexcept
    {
      return _end.Container.get();
    }

    KRYS_NODISCARD uint64 EndOffset() const noexcept
    {
      return _end.Offset;
    }
  };
}