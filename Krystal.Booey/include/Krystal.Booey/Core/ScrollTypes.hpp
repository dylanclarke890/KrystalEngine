#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

// FIXME: The rubber-banding and snapping types come with rubber-banding and scroll snapping.

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

  enum ScrollLogicalDirection : uint8
  {
    ScrollBlockDirectionBackward,
    ScrollBlockDirectionForward,
    ScrollInlineDirectionBackward,
    ScrollInlineDirectionForward
  };

  KRYS_NODISCARD constexpr ScrollDirection LogicalToPhysical(ScrollLogicalDirection direction,
                                                             bool isVertical, bool isFlipped) noexcept
  {
    switch (direction)
    {
      case ScrollBlockDirectionBackward:
      {
        if (isVertical)
        {
          return !isFlipped ? ScrollDirection::ScrollUp : ScrollDirection::ScrollDown;
        }
        return !isFlipped ? ScrollDirection::ScrollLeft : ScrollDirection::ScrollRight;
      }
      case ScrollBlockDirectionForward:
      {
        if (isVertical)
        {
          return !isFlipped ? ScrollDirection::ScrollDown : ScrollDirection::ScrollUp;
        }
        return !isFlipped ? ScrollDirection::ScrollRight : ScrollDirection::ScrollLeft;
      }
      case ScrollInlineDirectionBackward:
      {
        if (isVertical)
        {
          return !isFlipped ? ScrollDirection::ScrollLeft : ScrollDirection::ScrollRight;
        }
        return !isFlipped ? ScrollDirection::ScrollUp : ScrollDirection::ScrollDown;
      }
      case ScrollInlineDirectionForward:
      {
        if (isVertical)
        {
          return !isFlipped ? ScrollDirection::ScrollRight : ScrollDirection::ScrollLeft;
        }
        return !isFlipped ? ScrollDirection::ScrollDown : ScrollDirection::ScrollUp;
      }
    }
    return ScrollDirection::ScrollUp;
  }

  enum class ScrollEventAxis : uint8
  {
    Horizontal,
    Vertical
  };

  KRYS_NODISCARD constexpr ScrollEventAxis AxisFromDirection(ScrollDirection direction) noexcept
  {
    switch (direction)
    {
      case ScrollDirection::ScrollUp:    return ScrollEventAxis::Vertical;
      case ScrollDirection::ScrollDown:  return ScrollEventAxis::Vertical;
      case ScrollDirection::ScrollLeft:  return ScrollEventAxis::Horizontal;
      case ScrollDirection::ScrollRight: return ScrollEventAxis::Horizontal;
    }
    return ScrollEventAxis::Vertical;
  }

  /// @brief The parts of a scrollbar, as a bit set (`ScrollbarControlPartMask`).
  enum ScrollbarPart : uint32
  {
    NoPart = 0,
    BackButtonStartPart = 1,
    ForwardButtonStartPart = 1 << 1,
    BackTrackPart = 1 << 2,
    ThumbPart = 1 << 3,
    ForwardTrackPart = 1 << 4,
    BackButtonEndPart = 1 << 5,
    ForwardButtonEndPart = 1 << 6,
    ScrollbarBGPart = 1 << 7,
    TrackBGPart = 1 << 8,
    AllParts = 0xff'ff'ff'ff
  };

  using ScrollbarControlPartMask = uint32;

  enum ScrollbarButtonsPlacement : uint8
  {
    ScrollbarButtonsNone,
    ScrollbarButtonsSingle,
    ScrollbarButtonsDoubleStart,
    ScrollbarButtonsDoubleEnd,
    ScrollbarButtonsDoubleBoth
  };

  /// @brief What pressing a scrollbar part does.
  enum class ScrollbarButtonPressAction : uint8
  {
    None,
    CenterOnThumb,
    StartDrag,
    Scroll
  };
}
