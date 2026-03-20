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
    StaticRange(const StaticRangeInit &init) noexcept : AbstractRange(init.Start, init.End)
    {
      // TODO(fix): If init["startContainer"] or init["endContainer"] is a DocumentType or Attr node, then
      // throw an "InvalidNodeTypeError" DOMException.
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