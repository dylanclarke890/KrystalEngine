#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  namespace css
  {
    enum class ValueId : uint16;
  }

  namespace style
  {
    // <timeline-range-name> = cover | contain | entry | exit | entry-crossing | exit-crossing
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-cover
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-contain
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry-crossing
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit-crossing

    enum class SingleAnimationRangeName : uint8
    {
      Normal,
      Omitted,
      Cover,
      Contain,
      Entry,
      Exit,
      EntryCrossing,
      ExitCrossing
    };

    KRYS_NODISCARD css::ValueId ConvertSingleAnimationRangeNameToValueId(SingleAnimationRangeName) noexcept;
    KRYS_NODISCARD SingleAnimationRangeName ConvertValueIdToSingleAnimationRangeName(css::ValueId) noexcept;

    KRYS_NODISCARD css::CSSOMString
      ConvertSingleAnimationRangeNameToRangeString(SingleAnimationRangeName) noexcept;
    KRYS_NODISCARD SingleAnimationRangeName
      ConvertRangeStringToSingleTimelineRangeName(const css::CSSOMString &) noexcept;
  }
}
