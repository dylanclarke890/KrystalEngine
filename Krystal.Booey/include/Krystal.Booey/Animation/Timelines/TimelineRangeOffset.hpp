#pragma once

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"

namespace krys::boo
{
  /// @brief An offset within a named timeline range (`cover 20%`).
  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-timelinerangeoffset
  struct TimelineRangeOffset
  {
    dom::DOMString RangeName;
    RefPtr<css::CSSNumericValue> Offset;

    KRYS_NODISCARD bool operator==(const TimelineRangeOffset &) const noexcept = default;
  };
}
