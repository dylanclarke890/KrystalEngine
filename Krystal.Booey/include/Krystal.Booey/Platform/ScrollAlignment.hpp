#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief How to align a rect when scrolling it into view, depending on whether it's already visible,
  /// partly visible or hidden.
  struct ScrollAlignment
  {
    enum class Behavior : uint8
    {
      NoScroll,
      AlignCenter,
      AlignTop,
      AlignBottom,
      AlignLeft,
      AlignRight,
      AlignToClosestEdge
    };

    Behavior RectVisible;
    Behavior RectHidden;
    Behavior RectPartial;
    bool EnableLegacyHorizontalVisibilityThreshold {true};

    KRYS_NODISCARD Behavior GetVisibleBehavior() const noexcept
    {
      return RectVisible;
    }
    KRYS_NODISCARD Behavior GetPartialBehavior() const noexcept
    {
      return RectPartial;
    }
    KRYS_NODISCARD Behavior GetHiddenBehavior() const noexcept
    {
      return RectHidden;
    }

    void DisableLegacyHorizontalVisibilityThreshold() noexcept
    {
      EnableLegacyHorizontalVisibilityThreshold = false;
    }
    KRYS_NODISCARD bool LegacyHorizontalVisibilityThresholdEnabled() const noexcept
    {
      return EnableLegacyHorizontalVisibilityThreshold;
    }

    static const ScrollAlignment NoScroll;
    static const ScrollAlignment AlignCenterIfNotVisible;
    static const ScrollAlignment AlignToEdgeIfNotVisible;
    static const ScrollAlignment AlignCenterIfNeeded;
    static const ScrollAlignment AlignToEdgeIfNeeded;
    static const ScrollAlignment AlignCenterAlways;
    static const ScrollAlignment AlignTopAlways;
    static const ScrollAlignment AlignRightAlways;
    static const ScrollAlignment AlignLeftAlways;
    static const ScrollAlignment AlignBottomAlways;
  };

  /// @brief The part of a scroller's contents to make visible so @p exposeRect is aligned as asked, as a rect
  /// with the size of @p visibleBounds.
  KRYS_NODISCARD LayoutRect GetRectToExposeForScrollIntoView(
    const LayoutRect &visibleBounds, const LayoutRect &exposeRect, const ScrollAlignment &alignX,
    const ScrollAlignment &alignY, const Maybe<LayoutRect> visibilityCheckRect = null) noexcept;
}
