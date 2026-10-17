#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

// FIXME: The wheel, keyboard, rubber-banding and snapping types, and the scrollbar parts, come with scrolling
// input and painting.

namespace krys::boo
{
  // scrollPosition is in content coordinates (0,0 is at scrollOrigin), so may have negative components.
  using ScrollPosition = IntPoint;
  // scrollOffset() is the value used by scrollbars (min is 0,0), and should never have negative components.
  using ScrollOffset = IntPoint;

  enum class ScrollType : bool
  {
    User,
    Programmatic
  };

  enum class OverscrollBehavior : uint8
  {
    Auto,
    Contain,
    None
  };

  enum class ScrollDirection : uint8
  {
    ScrollUp,
    ScrollDown,
    ScrollLeft,
    ScrollRight
  };

  enum class ScrollAnimationStatus : uint8
  {
    NotAnimating,
    Animating,
  };

  enum class ScrollIsAnimated : bool
  {
    No,
    Yes
  };

  enum class OverflowAnchor : bool
  {
    Auto,
    None
  };

  enum class ScrollGranularity : uint8
  {
    Line,
    Page,
    Document,
    Pixel
  };

  enum class ScrollElasticity : uint8
  {
    Automatic,
    None,
    Allowed
  };

  enum class ScrollbarOrientation : uint8
  {
    Horizontal,
    Vertical
  };

  enum class ScrollbarWidth : uint8
  {
    Auto,
    Thin,
    None
  };

  enum class NativeScrollbarVisibility : uint8
  {
    Visible,
    HiddenByStyle,
    ReplacedByCustomScrollbar
  };

  enum class OverlayScrollbarSizeRelevancy : bool
  {
    IgnoreOverlayScrollbarSize,
    IncludeOverlayScrollbarSize
  };

  enum class ScrollbarOverlayStyle : uint8
  {
    Default,
    Dark,
    Light
  };

  enum class ScrollClamping : bool
  {
    Unclamped,
    Clamped
  };

  // This value controls the method used to select snap points during scrolling. This may either
  // be "directional" or "closest." The directional method only chooses snap points that are at or
  // beyond the scroll destination in the direction of the scroll. The "closest" method does not
  // have this constraint.
  enum class ScrollSnapPointSelectionMethod : uint8
  {
    Directional,
    Closest,
  };

  enum class ScrollbarMode : uint8
  {
    Auto,
    AlwaysOff,
    AlwaysOn
  };

  /// @brief Whether scrolling something into view scrolls, and how far up the frames it goes.
  enum class SelectionRevealMode : uint8
  {
    Reveal,
    RevealUpToMainFrame,
    DelegateMainFrameScroll,
    DoNotReveal
  };

  enum class AllowScrollingOverflowHidden : bool
  {
    No,
    Yes
  };

  struct ScrollPositionChangeOptions
  {
    ScrollType Type;
    ScrollClamping Clamping = ScrollClamping::Clamped;
    ScrollIsAnimated Animated = ScrollIsAnimated::No;
    ScrollSnapPointSelectionMethod SnapPointSelectionMethod = ScrollSnapPointSelectionMethod::Closest;

    KRYS_NODISCARD static ScrollPositionChangeOptions CreateProgrammatic() noexcept
    {
      return {ScrollType::Programmatic};
    }

    KRYS_NODISCARD static ScrollPositionChangeOptions CreateUser() noexcept
    {
      return {ScrollType::User};
    }

    KRYS_NODISCARD static ScrollPositionChangeOptions CreateProgrammaticUnclamped() noexcept
    {
      return {ScrollType::Programmatic, ScrollClamping::Unclamped};
    }
  };

  KRYS_NODISCARD inline int32 OffsetForOrientation(ScrollOffset offset,
                                                   ScrollbarOrientation orientation) noexcept
  {
    switch (orientation)
    {
      case ScrollbarOrientation::Horizontal: return offset.X();
      case ScrollbarOrientation::Vertical:   return offset.Y();
    }
    krys_debug_assert(false);
    return 0;
  }
}
