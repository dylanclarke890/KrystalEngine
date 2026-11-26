#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutRect.hpp"

namespace krys::web
{
  /// @brief How to scroll a rect into view along one axis, depending on how much of it is already visible.
  struct ScrollAlignment
  {
    /// @brief Where to put the rect along the axis.
    enum class Behavior : uint8
    {
      /// @brief Leave the scroll position as it is.
      NoScroll,
      AlignCenter,
      AlignTop,
      AlignBottom,
      AlignLeft,
      AlignRight,
      /// @brief Align the edge it's nearest to scroll to, as `nearest` does in `scrollIntoView()`
      /// (https://drafts.csswg.org/cssom-view-1/#determine-the-scroll-into-view-position).
      AlignToClosestEdge,
    };

    /// @brief The behaviour when the rect is all visible (or covers the visible area).
    Behavior RectVisible;
    /// @brief The behaviour when none of it is visible.
    Behavior RectHidden;
    /// @brief The behaviour when some of it is visible.
    Behavior RectPartial;
    /// @brief Whether a rect with at least 32px visible horizontally counts as visible, so revealing it
    /// needn't scroll sideways. `scrollIntoView()` turns this off; revealing the selection or focus keeps it.
    bool EnableLegacyHorizontalVisibilityThreshold {true};

    void DisableLegacyHorizontalVisibilityThreshold() noexcept
    {
      EnableLegacyHorizontalVisibilityThreshold = false;
    }

    /// @brief Never scroll.
    static const ScrollAlignment NoScroll;
    /// @brief Centre a hidden rect; leave a visible or partly visible one.
    static const ScrollAlignment AlignCenterIfNotVisible;
    /// @brief Align a hidden rect to the nearest edge; leave a visible or partly visible one.
    static const ScrollAlignment AlignToEdgeIfNotVisible;
    /// @brief Centre a hidden rect and align a partly visible one to the nearest edge.
    static const ScrollAlignment AlignCenterIfNeeded;
    /// @brief Align a hidden or partly visible rect to the nearest edge.
    static const ScrollAlignment AlignToEdgeIfNeeded;
    /// @brief Always centre, even when the rect is visible; the next four always align to one edge.
    static const ScrollAlignment AlignCenterAlways;
    static const ScrollAlignment AlignTopAlways;
    static const ScrollAlignment AlignRightAlways;
    static const ScrollAlignment AlignLeftAlways;
    static const ScrollAlignment AlignBottomAlways;
  };

  /// @brief The part of a scroller's contents to make visible so @p exposeRect is aligned as asked, as a rect
  /// with the size of @p visibleBounds.
  /// @param visibilityCheckRect The rect whose overlap with @p visibleBounds says whether @p exposeRect
  /// counts as hidden; @p exposeRect itself if not given.
  KRYS_NODISCARD LayoutRect GetRectToExposeForScrollIntoView(
    const LayoutRect &visibleBounds, const LayoutRect &exposeRect, const ScrollAlignment &alignX,
    const ScrollAlignment &alignY, const Maybe<LayoutRect> visibilityCheckRect = null) noexcept;
}
