#pragma once

#include "Krystal.Booey/Animation/TimelineRangeOffset.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  namespace style
  {
    enum class SingleAnimationRangeType : bool;
  }

  /// @brief An edge of an animation's attachment range, as script gives it (`rangeStart`, `rangeEnd`).
  /// @see https://drafts.csswg.org/web-animations-2/#dom-animation-rangestart
  using TimelineRangeValue =
    Variant<TimelineRangeOffset, RefPtr<css::CSSNumericValue>, RefPtr<css::CSSKeywordValue>, dom::DOMString>;

  /// @brief Parses @p value as an edge of an attachment range for @p element's document, or null if it isn't
  /// one.
  KRYS_NODISCARD RefPtr<css::Value> ConvertToCSSValue(TimelineRangeValue &&value,
                                                      RefPtr<dom::Element> element,
                                                      style::SingleAnimationRangeType type) noexcept;
}
