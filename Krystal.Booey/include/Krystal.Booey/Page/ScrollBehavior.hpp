#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  /// @brief How a programmatic scroll moves.
  /// @see https://drafts.csswg.org/cssom-view/#enumdef-scrollbehavior
  enum class ScrollBehavior : uint8
  {
    Auto,
    Instant,
    Smooth
  };

  /// @brief Whether a scroll with @p behavior, of the scrolling box of @p associatedElement, is smooth.
  /// @see https://drafts.csswg.org/cssom-view/#scrolling
  KRYS_NODISCARD bool UseSmoothScrolling(ScrollBehavior behavior,
                                         const dom::Element *associatedElement) noexcept;
}
