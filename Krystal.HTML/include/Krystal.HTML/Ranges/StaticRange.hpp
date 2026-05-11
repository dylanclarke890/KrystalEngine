#pragma once

#include "Krystal.HTML/Ranges/AbstractRange.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"

namespace Krys::HTML
{
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

  class StaticRange : public AbstractRange
  {
  public:
    StaticRange(const StaticRangeInit &init) noexcept : AbstractRange(init.Start(), init.End())
    {
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      Node &startContainer = *this->StartContainer();
      Node &endContainer = *this->EndContainer();

      if (!TreeQueries::IsConnectedInSameTreeScope(*this->StartContainer(), *this->EndContainer()))
      {
        return false;
      }

      if (StartOffset() > TreeQueries::Length(startContainer))
      {
        return false;
      }

      if (EndOffset() > TreeQueries::Length(endContainer))
      {
        return false;
      }

      if (&startContainer == &endContainer)
      {
        return EndOffset() > StartOffset();
      }

      return !StrongOrder::IsGreaterThan(_start.ComparePositionTo(_end));
    }
  };
}