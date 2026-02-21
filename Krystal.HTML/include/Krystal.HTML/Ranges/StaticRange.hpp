#pragma once

#include "Krystal.HTML/Ranges/AbstractRange.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"

namespace Krys::HTML
{
  struct StaticRangeInit
  {
    BoundaryPoint Start;
    BoundaryPoint End;
  };

  class StaticRange : public AbstractRange
  {
  public:
    constexpr StaticRange(const StaticRangeInit &init) noexcept : AbstractRange(init.Start, init.End)
    {
    }

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      Node &startContainer = *this->StartContainer();
      Node &endContainer = *this->EndContainer();

      if (!NodeQueries::IsConnectedInSameTreeScope(*this->StartContainer(), *this->EndContainer()))
      {
        return false;
      }

      if (StartOffset() > startContainer.Length())
      {
        return false;
      }

      if (EndOffset() > endContainer.Length())
      {
        return false;
      }

      if (&startContainer == &endContainer)
      {
        return EndOffset() > StartOffset();
      }

      return !StrongOrder::IsGreaterThan(_start <=> _end);
    }
  };
}