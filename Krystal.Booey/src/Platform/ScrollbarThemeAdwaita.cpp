#include "Krystal.Booey/Platform/ScrollbarThemeAdwaita.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr int32 ScrollbarSize = 21;
    constexpr int32 ScrollbarBorderSize = 1;
    constexpr int32 ThumbBorderSize = 1;
    constexpr int32 OverlayThumbSize = 3;
    constexpr int32 MinimumThumbSize = 40;
    constexpr int32 HorizThumbMargin = 6;
    constexpr int32 HorizOverlayThumbMargin = 3;
    constexpr int32 VertThumbMargin = 7;

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

  bool ScrollbarThemeAdwaita::UsesOverlayScrollbars() const noexcept
  {
    // Classic scrollbars (decided): overlay scrollbars fade in and out, which needs animations.
    // FIXME: Overlay scrollbars.
    return false;
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
    return ScrollbarSize;
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

    SRGBA<uint8> overlayThumbBorderColor;
    SRGBA<uint8> overlayTroughBorderColor;
    SRGBA<uint8> overlayTroughColor;
    SRGBA<uint8> thumbHoveredColor;
    SRGBA<uint8> thumbPressedColor;
    SRGBA<uint8> thumbColor;

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

    GraphicsContextStateSaver stateSaver(graphicsContext);
    if (opacity != 1)
    {
      graphicsContext.Clip(FloatRect(damageRect));
      graphicsContext.BeginTransparencyLayer(static_cast<float>(opacity));
    }

    int32 thumbSize = ScrollbarSize - ScrollbarBorderSize - HorizThumbMargin * 2;
    bool onLeft = scrollbar.GetScrollableArea().ShouldPlaceVerticalScrollbarOnLeft();

    // Classic scrollbars have no track or border: what's behind them (the box's background, or the page's)
    // shows through.
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
          trough.Move(ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2) - ScrollbarBorderSize,
                      VertThumbMargin);
        }
        else
        {
          trough.Move(ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2), VertThumbMargin);
        }
        trough.SetWidth(thumbSize);
        trough.SetHeight(rect.Height() - VertThumbMargin * 2);
      }
      else
      {
        trough.Move(VertThumbMargin, ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2));
        trough.SetWidth(rect.Width() - VertThumbMargin * 2);
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
          thumb.Move(HorizOverlayThumbMargin, thumbPos + VertThumbMargin);
        }
        else
        {
          thumb.Move(ScrollbarSize - OverlayThumbSize - HorizOverlayThumbMargin, thumbPos + VertThumbMargin);
        }
        thumb.SetWidth(OverlayThumbSize);
        thumb.SetHeight(thumbLen - VertThumbMargin * 2);
      }
      else
      {
        thumb.Move(thumbPos + VertThumbMargin, ScrollbarSize - OverlayThumbSize - HorizOverlayThumbMargin);
        thumb.SetWidth(thumbLen - VertThumbMargin * 2);
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
          thumb.Move(ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2) - ScrollbarBorderSize,
                     thumbPos + VertThumbMargin);
        }
        else
        {
          thumb.Move(ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2), thumbPos + VertThumbMargin);
        }
        thumb.SetWidth(thumbSize);
        thumb.SetHeight(thumbLen - VertThumbMargin * 2);
      }
      else
      {
        thumb.Move(thumbPos + VertThumbMargin, ScrollbarSize - (ScrollbarSize / 2 + thumbSize / 2));
        thumb.SetWidth(thumbLen - VertThumbMargin * 2);
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

  void ScrollbarThemeAdwaita::PaintScrollCorner(ScrollableArea &, GraphicsContext &, const IntRect &) noexcept
  {
    // The corner is left unpainted, as the scrollbars' tracks are: what's behind it shows through.
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
