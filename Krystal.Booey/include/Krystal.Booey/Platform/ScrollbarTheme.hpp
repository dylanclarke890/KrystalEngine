#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <limits>

namespace krys::boo
{
  class GraphicsContext;
  class PlatformMouseEvent;
  class ScrollableArea;
  class Scrollbar;

  /// @brief How scrollbars look, how much space they take, which part of one a point is over, and what
  /// pressing a part does.
  /// @note The theme is the Adwaita one (`ScrollbarThemeAdwaita`), drawn through `GraphicsContext`.
  // FIXME: Control tints, overlay scrollbar styles, tickmarks and overhang areas, once there are those.
  class ScrollbarTheme
  {
    KRYS_NON_COPY_MOVABLE(ScrollbarTheme);

  public:
    ScrollbarTheme() noexcept = default;
    virtual ~ScrollbarTheme() noexcept = default;

    KRYS_NODISCARD static ScrollbarTheme &Theme() noexcept;

    virtual void UpdateEnabledState(Scrollbar &) noexcept
    {
    }

    /// @return Whether the theme painted the scrollbar.
    virtual bool Paint(Scrollbar &, GraphicsContext &, const IntRect & /*damageRect*/) noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual ScrollbarPart HitTest(Scrollbar &, const IntPoint &) noexcept
    {
      return NoPart;
    }

    KRYS_NODISCARD virtual int32
      ScrollbarThickness(ScrollbarWidth = ScrollbarWidth::Auto,
                         OverlayScrollbarSizeRelevancy =
                           OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize) const noexcept
    {
      return 0;
    }

    KRYS_NODISCARD virtual ScrollbarButtonsPlacement ButtonsPlacement() const noexcept
    {
      return ScrollbarButtonsSingle;
    }

    KRYS_NODISCARD virtual bool UsesOverlayScrollbars() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool InvalidateOnMouseEnterExit() const noexcept
    {
      return false;
    }

    /// @brief How long a press on the track or a button waits before it repeats, and then between repeats.
    KRYS_NODISCARD virtual MonotonicTime::duration InitialAutoscrollTimerDelay() const noexcept
    {
      return std::chrono::milliseconds(250);
    }
    KRYS_NODISCARD virtual MonotonicTime::duration AutoscrollTimerDelay() const noexcept
    {
      return std::chrono::milliseconds(50);
    }

    void InvalidateParts(Scrollbar &scrollbar, ScrollbarControlPartMask mask) noexcept
    {
      if (mask & BackButtonStartPart)
      {
        InvalidatePart(scrollbar, BackButtonStartPart);
      }
      if (mask & ForwardButtonStartPart)
      {
        InvalidatePart(scrollbar, ForwardButtonStartPart);
      }
      if (mask & BackTrackPart)
      {
        InvalidatePart(scrollbar, BackTrackPart);
      }
      if (mask & ThumbPart)
      {
        InvalidatePart(scrollbar, ThumbPart);
      }
      if (mask & ForwardTrackPart)
      {
        InvalidatePart(scrollbar, ForwardTrackPart);
      }
      if (mask & BackButtonEndPart)
      {
        InvalidatePart(scrollbar, BackButtonEndPart);
      }
      if (mask & ForwardButtonEndPart)
      {
        InvalidatePart(scrollbar, ForwardButtonEndPart);
      }
    }

    virtual void InvalidatePart(Scrollbar &, ScrollbarPart) noexcept
    {
    }

    virtual void PaintScrollCorner(ScrollableArea &area, GraphicsContext &context,
                                   const IntRect &cornerRect) noexcept
    {
      DefaultPaintScrollCorner(area, context, cornerRect);
    }

    static void DefaultPaintScrollCorner(ScrollableArea &, GraphicsContext &context,
                                         const IntRect &cornerRect) noexcept;

    KRYS_NODISCARD virtual ScrollbarButtonPressAction
      HandleMousePressEvent(Scrollbar &, const PlatformMouseEvent &, ScrollbarPart) noexcept;

    KRYS_NODISCARD virtual bool ShouldSnapBackToDragOrigin(Scrollbar &, const PlatformMouseEvent &) noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool ShouldDragDocumentInsteadOfThumb(Scrollbar &,
                                                                 const PlatformMouseEvent &) noexcept
    {
      return false;
    }

    /// @brief The position of the thumb relative to the track.
    KRYS_NODISCARD virtual int32 ThumbPosition(Scrollbar &) noexcept
    {
      return 0;
    }

    /// @brief The length of the thumb along the axis of the scrollbar.
    KRYS_NODISCARD virtual int32 ThumbLength(Scrollbar &) noexcept
    {
      return 0;
    }

    /// @brief The position of the track relative to the scrollbar.
    KRYS_NODISCARD virtual int32 TrackPosition(Scrollbar &) noexcept
    {
      return 0;
    }

    /// @brief The length of the track along the axis of the scrollbar.
    KRYS_NODISCARD virtual int32 TrackLength(Scrollbar &) noexcept
    {
      return 0;
    }

    KRYS_NODISCARD virtual int32 MaxOverlapBetweenPages() const noexcept
    {
      return std::numeric_limits<int32>::max();
    }

    KRYS_NODISCARD virtual bool IsMockTheme() const noexcept
    {
      return false;
    }
  };
}
