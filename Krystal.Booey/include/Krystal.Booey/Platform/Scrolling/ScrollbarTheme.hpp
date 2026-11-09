/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
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
  // FIXME: Control tints and overlay scrollbar styles, once there are those. (There are no find-in-page
  // tickmarks; see DIFFERENCES.md.)
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
