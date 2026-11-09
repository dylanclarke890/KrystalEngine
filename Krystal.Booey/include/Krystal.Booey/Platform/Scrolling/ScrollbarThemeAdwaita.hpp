/*
 * Copyright (C) 2014, 2020 Igalia S.L.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Platform/Scrolling/ScrollbarThemeComposite.hpp"

namespace krys::boo
{
  /// @brief GNOME's Adwaita scrollbars, drawn through `GraphicsContext`: a track with a border along its
  /// inner edge and a rounded thumb, without buttons.
  /// @note The scrollbars are classic by default, as Chrome's on Windows: always shown and taking layout
  /// space. A host can choose overlay scrollbars instead, which take no space, draw over the content, and
  /// fade in while the mouse moves over their area or it scrolls.
  class ScrollbarThemeAdwaita : public ScrollbarThemeComposite
  {
  private:
    bool _usesOverlayScrollbars {false};

  public:
    ScrollbarThemeAdwaita() noexcept = default;
    ~ScrollbarThemeAdwaita() noexcept override = default;

    /// @brief Chooses overlay scrollbars (@p overlay) or classic ones, for every page: the pages showing
    /// change over, laying out again for the space their scrollbars take.
    static void SetUsesOverlayScrollbars(bool overlay) noexcept;

  protected:
    KRYS_NODISCARD bool UsesOverlayScrollbars() const noexcept override;
    KRYS_NODISCARD bool InvalidateOnMouseEnterExit() const noexcept override
    {
      return UsesOverlayScrollbars();
    }
    bool Paint(Scrollbar &, GraphicsContext &, const IntRect &) noexcept override;
    void PaintScrollCorner(ScrollableArea &, GraphicsContext &, const IntRect &) noexcept override;
    KRYS_NODISCARD ScrollbarButtonPressAction HandleMousePressEvent(Scrollbar &, const PlatformMouseEvent &,
                                                                    ScrollbarPart) noexcept override;
    KRYS_NODISCARD int32 ScrollbarThickness(
      ScrollbarWidth, OverlayScrollbarSizeRelevancy =
                        OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize) const noexcept override;
    KRYS_NODISCARD int32 MinimumThumbLength(Scrollbar &) noexcept override;
    KRYS_NODISCARD bool HasButtons(Scrollbar &) noexcept override;
    KRYS_NODISCARD bool HasThumb(Scrollbar &) noexcept override;
    KRYS_NODISCARD IntRect BackButtonRect(Scrollbar &, ScrollbarPart,
                                          bool painting = false) noexcept override;
    KRYS_NODISCARD IntRect ForwardButtonRect(Scrollbar &, ScrollbarPart,
                                             bool painting = false) noexcept override;
    KRYS_NODISCARD IntRect TrackRect(Scrollbar &, bool painting = false) noexcept override;
  };
}
