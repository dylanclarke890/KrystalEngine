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

#include "Krystal.Booey/Platform/Scrolling/ScrollbarThemeAdwaita.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief A scrollbar's thickness, and its thumb's margins: across the scrollbar, and at its ends.
    struct ScrollbarMetrics
    {
      int32 Size;
      int32 ThumbMargin;
      int32 ThumbEndMargin;
    };

    constexpr ScrollbarMetrics AutoMetrics {.Size = 21, .ThumbMargin = 6, .ThumbEndMargin = 7};
    // A thin scrollbar is a little over half as thick, with a thumb 6px across rather than 8px.
    constexpr ScrollbarMetrics ThinMetrics {.Size = 13, .ThumbMargin = 3, .ThumbEndMargin = 4};

    KRYS_NODISCARD constexpr const ScrollbarMetrics &MetricsFor(ScrollbarWidth width) noexcept
    {
      return width == ScrollbarWidth::Thin ? ThinMetrics : AutoMetrics;
    }

    constexpr int32 ScrollbarBorderSize = 1;
    constexpr int32 ThumbBorderSize = 1;
    constexpr int32 OverlayThumbSize = 3;
    constexpr int32 MinimumThumbSize = 40;
    constexpr int32 HorizOverlayThumbMargin = 3;

    /// @brief A thumb colour's shade for hovering and pressing: lighter for a dark colour, darker for a light
    /// one.
    KRYS_NODISCARD Color ThumbStateColor(const Color &color, bool darkThumb) noexcept
    {
      return darkThumb ? color.Lightened() : color.Darkened();
    }

    constexpr auto OverlayThumbBorderColorLight = SRGBA<uint8> {255, 255, 255, 102};
    constexpr auto OverlayTroughColorLight = SRGBA<uint8> {0, 0, 0, 25};
    constexpr auto ThumbHoveredColorLight = SRGBA<uint8> {0, 0, 0, 102};
    constexpr auto ThumbPressedColorLight = SRGBA<uint8> {0, 0, 0, 153};
    constexpr auto ThumbColorLight = SRGBA<uint8> {0, 0, 0, 51};

    constexpr auto OverlayThumbBorderColorDark = SRGBA<uint8> {0, 0, 0, 51};
    constexpr auto OverlayTroughColorDark = SRGBA<uint8> {255, 255, 255, 25};
    constexpr auto ThumbHoveredColorDark = SRGBA<uint8> {255, 255, 255, 102};
    constexpr auto ThumbPressedColorDark = SRGBA<uint8> {255, 255, 255, 153};
    constexpr auto ThumbColorDark = SRGBA<uint8> {255, 255, 255, 51};
  }

  void ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(bool overlay) noexcept
  {
    auto &theme = static_cast<ScrollbarThemeAdwaita &>(Theme());
    if (theme._usesOverlayScrollbars == overlay)
    {
      return;
    }
    theme._usesOverlayScrollbars = overlay;

    Page::ForEachPage(
      [](Page &page)
      {
        if (auto *view = page.MainFrame().View())
        {
          view->ForEachScrollableArea([](ScrollableArea &area) { area.ScrollbarStyleChanged(); });
        }
        page.ScheduleRenderingUpdate();
      });
  }

  bool ScrollbarThemeAdwaita::UsesOverlayScrollbars() const noexcept
  {
    return _usesOverlayScrollbars;
  }

  int32
    ScrollbarThemeAdwaita::ScrollbarThickness(ScrollbarWidth scrollbarWidth,
                                              OverlayScrollbarSizeRelevancy overlayRelevancy) const noexcept
  {
    if (scrollbarWidth == ScrollbarWidth::None
        || (UsesOverlayScrollbars()
            && overlayRelevancy == OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize))
    {
      return 0;
    }
    return MetricsFor(scrollbarWidth).Size;
  }

  int32 ScrollbarThemeAdwaita::MinimumThumbLength(Scrollbar &) noexcept
  {
    return MinimumThumbSize;
  }

  bool ScrollbarThemeAdwaita::HasButtons(Scrollbar &) noexcept
  {
    return false;
  }

  bool ScrollbarThemeAdwaita::HasThumb(Scrollbar &scrollbar) noexcept
  {
    return ThumbLength(scrollbar) > 0;
  }

  IntRect ScrollbarThemeAdwaita::BackButtonRect(Scrollbar &, ScrollbarPart, bool) noexcept
  {
    return {};
  }

  IntRect ScrollbarThemeAdwaita::ForwardButtonRect(Scrollbar &, ScrollbarPart, bool) noexcept
  {
    return {};
  }

  IntRect ScrollbarThemeAdwaita::TrackRect(Scrollbar &scrollbar, bool) noexcept
  {
    return scrollbar.FrameRect();
  }

  bool ScrollbarThemeAdwaita::Paint(Scrollbar &scrollbar, GraphicsContext &graphicsContext,
                                    const IntRect &damageRect) noexcept
  {
    // FIXME: Painting disabled contexts, once there are those.
    if (!scrollbar.Enabled() && UsesOverlayScrollbars())
    {
      return true;
    }

    IntRect rect = scrollbar.FrameRect();
    if (!rect.Intersects(damageRect))
    {
      return true;
    }

    double opacity = UsesOverlayScrollbars() ? scrollbar.Opacity() : 1;
    if (!opacity)
    {
      return true;
    }

    Color overlayThumbBorderColor;
    Color overlayTroughBorderColor;
    Color overlayTroughColor;
    Color thumbHoveredColor;
    Color thumbPressedColor;
    Color thumbColor;

    if (scrollbar.GetScrollableArea().UseDarkAppearanceForScrollbars())
    {
      overlayThumbBorderColor = OverlayThumbBorderColorDark;
      overlayTroughBorderColor = OverlayThumbBorderColorDark;
      overlayTroughColor = OverlayTroughColorDark;
      thumbHoveredColor = ThumbHoveredColorDark;
      thumbPressedColor = ThumbPressedColorDark;
      thumbColor = ThumbColorDark;
    }
    else
    {
      overlayThumbBorderColor = OverlayThumbBorderColorLight;
      overlayTroughBorderColor = OverlayThumbBorderColorLight;
      overlayTroughColor = OverlayTroughColorLight;
      thumbHoveredColor = ThumbHoveredColorLight;
      thumbPressedColor = ThumbPressedColorLight;
      thumbColor = ThumbColorLight;
    }

    // scrollbar-color gives the thumb its colour, shaded while it's hovered and more while it's pressed.
    auto scrollbarColor = scrollbar.GetScrollableArea().ScrollbarColorStyle();
    if (scrollbarColor)
    {
      // Darker than mid grey.
      bool darkThumb = scrollbarColor->ThumbColor.Luminance() < 0.2;
      thumbColor = scrollbarColor->ThumbColor;
      thumbHoveredColor = ThumbStateColor(thumbColor, darkThumb);
      thumbPressedColor = ThumbStateColor(thumbHoveredColor, darkThumb);
    }

    GraphicsContextStateSaver stateSaver(graphicsContext);
    if (opacity != 1)
    {
      graphicsContext.Clip(FloatRect(damageRect));
      graphicsContext.BeginTransparencyLayer(static_cast<float>(opacity));
    }

    const auto &metrics = MetricsFor(scrollbar.WidthStyle());
    int32 thumbSize = metrics.Size - ScrollbarBorderSize - metrics.ThumbMargin * 2;
    bool onLeft = scrollbar.GetScrollableArea().ShouldPlaceVerticalScrollbarOnLeft();

    // scrollbar-color's track colour fills the track. Without it, classic scrollbars have no track or border:
    // what's behind them (the box's background, or the page's) shows through.
    if (scrollbarColor && !UsesOverlayScrollbars())
    {
      graphicsContext.FillRect(FloatRect(rect), scrollbarColor->TrackColor);
    }
    if (UsesOverlayScrollbars() && scrollbar.HoveredPart() != NoPart)
    {
      int32 thumbCornerSize = thumbSize / 2;
      FloatSize corner(static_cast<float>(thumbCornerSize), static_cast<float>(thumbCornerSize));
      FloatSize borderCorner(static_cast<float>(thumbCornerSize + ThumbBorderSize),
                             static_cast<float>(thumbCornerSize + ThumbBorderSize));

      IntRect trough = rect;
      if (scrollbar.Orientation() == ScrollbarOrientation::Vertical)
      {
        if (onLeft)
        {
          trough.Move(metrics.Size - (metrics.Size / 2 + thumbSize / 2) - ScrollbarBorderSize,
                      metrics.ThumbEndMargin);
        }
        else
        {
          trough.Move(metrics.Size - (metrics.Size / 2 + thumbSize / 2), metrics.ThumbEndMargin);
        }
        trough.SetWidth(thumbSize);
        trough.SetHeight(rect.Height() - metrics.ThumbEndMargin * 2);
      }
      else
      {
        trough.Move(metrics.ThumbEndMargin, metrics.Size - (metrics.Size / 2 + thumbSize / 2));
        trough.SetWidth(rect.Width() - metrics.ThumbEndMargin * 2);
        trough.SetHeight(thumbSize);
      }

      IntRect troughBorder(trough);
      troughBorder.Inflate(ThumbBorderSize);

      Path path;
      path.AddRoundedRect(FloatRect(trough), corner);
      graphicsContext.SetFillRule(WindRule::NonZero);
      graphicsContext.SetFillColor(overlayTroughColor);
      graphicsContext.FillPath(path);
      path.Clear();

      path.AddRoundedRect(FloatRect(trough), corner);
      path.AddRoundedRect(FloatRect(troughBorder), borderCorner);
      graphicsContext.SetFillRule(WindRule::EvenOdd);
      graphicsContext.SetFillColor(overlayTroughBorderColor);
      graphicsContext.FillPath(path);
    }

    int32 thumbCornerSize;
    int32 thumbPos = ThumbPosition(scrollbar);
    int32 thumbLen = ThumbLength(scrollbar);
    IntRect thumb = rect;
    if (scrollbar.HoveredPart() == NoPart && UsesOverlayScrollbars())
    {
      thumbCornerSize = OverlayThumbSize / 2;

      if (scrollbar.Orientation() == ScrollbarOrientation::Vertical)
      {
        if (onLeft)
        {
          thumb.Move(HorizOverlayThumbMargin, thumbPos + metrics.ThumbEndMargin);
        }
        else
        {
          thumb.Move(metrics.Size - OverlayThumbSize - HorizOverlayThumbMargin,
                     thumbPos + metrics.ThumbEndMargin);
        }
        thumb.SetWidth(OverlayThumbSize);
        thumb.SetHeight(thumbLen - metrics.ThumbEndMargin * 2);
      }
      else
      {
        thumb.Move(thumbPos + metrics.ThumbEndMargin,
                   metrics.Size - OverlayThumbSize - HorizOverlayThumbMargin);
        thumb.SetWidth(thumbLen - metrics.ThumbEndMargin * 2);
        thumb.SetHeight(OverlayThumbSize);
      }
    }
    else
    {
      thumbCornerSize = thumbSize / 2;

      if (scrollbar.Orientation() == ScrollbarOrientation::Vertical)
      {
        if (onLeft)
        {
          thumb.Move(metrics.Size - (metrics.Size / 2 + thumbSize / 2) - ScrollbarBorderSize,
                     thumbPos + metrics.ThumbEndMargin);
        }
        else
        {
          thumb.Move(metrics.Size - (metrics.Size / 2 + thumbSize / 2), thumbPos + metrics.ThumbEndMargin);
        }
        thumb.SetWidth(thumbSize);
        thumb.SetHeight(thumbLen - metrics.ThumbEndMargin * 2);
      }
      else
      {
        thumb.Move(thumbPos + metrics.ThumbEndMargin, metrics.Size - (metrics.Size / 2 + thumbSize / 2));
        thumb.SetWidth(thumbLen - metrics.ThumbEndMargin * 2);
        thumb.SetHeight(thumbSize);
      }
    }

    FloatSize corner(static_cast<float>(thumbCornerSize), static_cast<float>(thumbCornerSize));
    FloatSize borderCorner(static_cast<float>(thumbCornerSize + ThumbBorderSize),
                           static_cast<float>(thumbCornerSize + ThumbBorderSize));

    Path path;

    path.AddRoundedRect(FloatRect(thumb), corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    if (scrollbar.PressedPart() == ThumbPart)
    {
      graphicsContext.SetFillColor(thumbPressedColor);
    }
    else if (scrollbar.HoveredPart() == ThumbPart)
    {
      graphicsContext.SetFillColor(thumbHoveredColor);
    }
    else
    {
      graphicsContext.SetFillColor(thumbColor);
    }
    graphicsContext.FillPath(path);
    path.Clear();

    if (UsesOverlayScrollbars())
    {
      IntRect thumbBorder(thumb);
      thumbBorder.Inflate(ThumbBorderSize);

      path.AddRoundedRect(FloatRect(thumb), corner);
      path.AddRoundedRect(FloatRect(thumbBorder), borderCorner);
      graphicsContext.SetFillRule(WindRule::EvenOdd);
      graphicsContext.SetFillColor(overlayThumbBorderColor);
      graphicsContext.FillPath(path);
    }

    if (opacity != 1)
    {
      graphicsContext.EndTransparencyLayer();
    }

    return true;
  }

  void ScrollbarThemeAdwaita::PaintScrollCorner(ScrollableArea &area, GraphicsContext &context,
                                                const IntRect &cornerRect) noexcept
  {
    // The corner takes scrollbar-color's track colour. Without one, it's left unpainted, as the scrollbars'
    // tracks are: what's behind it shows through.
    if (auto scrollbarColor = area.ScrollbarColorStyle())
    {
      context.FillRect(FloatRect(cornerRect), scrollbarColor->TrackColor);
    }
  }

  ScrollbarButtonPressAction ScrollbarThemeAdwaita::HandleMousePressEvent(Scrollbar &,
                                                                          const PlatformMouseEvent &event,
                                                                          ScrollbarPart pressedPart) noexcept
  {
    bool warpSlider = false;
    switch (pressedPart)
    {
      case BackTrackPart:
      case ForwardTrackPart:
        // FIXME: The system's setting (primaryButtonWarpsSlider), once the host reports it; off, as on
        // platforms other than GTK and WPE. The shift key or middle/right button reverses the sense.
        if (event.ShiftKey() || event.Button() != MouseButton::Left)
        {
          warpSlider = !warpSlider;
        }
        return warpSlider ? ScrollbarButtonPressAction::CenterOnThumb : ScrollbarButtonPressAction::Scroll;
      case ThumbPart:
        if (event.Button() != MouseButton::Right)
        {
          return ScrollbarButtonPressAction::StartDrag;
        }
        break;
      case BackButtonStartPart:
      case ForwardButtonStartPart:
      case BackButtonEndPart:
      case ForwardButtonEndPart:   return ScrollbarButtonPressAction::Scroll;
      default:                     break;
    }

    return ScrollbarButtonPressAction::None;
  }
}
