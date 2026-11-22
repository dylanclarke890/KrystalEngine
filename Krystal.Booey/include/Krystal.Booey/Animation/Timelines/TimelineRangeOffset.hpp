#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class CSSNumericValue;

  /// @brief An offset within a named timeline range (`cover 20%`).
  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-timelinerangeoffset
  struct TimelineRangeOffset
  {
    DOMString RangeName;
    RefPtr<CSSNumericValue> Offset;

    KRYS_NODISCARD bool operator==(const TimelineRangeOffset &) const noexcept = default;
  };
}
