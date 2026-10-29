#include "Krystal.Booey/Rendering/EllipsisBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  EllipsisBoxPainter::EllipsisBoxPainter(const inline_iterator::LineBox &lineBox, PaintInfo &paintInfo,
                                         const LayoutPoint &paintOffset, Color selectionForegroundColor,
                                         Color selectionBackgroundColor) noexcept
      : _lineBox(lineBox), _paintInfo(paintInfo), _paintOffset(paintOffset),
        _selectionForegroundColor(selectionForegroundColor),
        _selectionBackgroundColor(selectionBackgroundColor)
  {
  }

  void EllipsisBoxPainter::Paint() noexcept
  {
    // FIXME: Transition it to TextPainter.
    auto &context = _paintInfo.Context();
    auto &lineStyle = _lineBox.Style();
    auto textColor = lineStyle.VisitedDependentColorWithColorFilter(css::PropertyId::WebkitTextFillColor);

    if (_paintInfo.ForceTextColor())
    {
      textColor = _paintInfo.ForcedTextColor();
    }

    if (_lineBox.EllipsisSelectionState() != RenderObject::HighlightState::None)
    {
      PaintSelection();

      // Select the correct color for painting the text.
      auto foreground =
        _paintInfo.ForceTextColor() ? _paintInfo.ForcedTextColor() : _selectionForegroundColor;
      if (foreground.IsValid() && foreground != textColor)
      {
        context.SetFillColor(foreground);
      }
    }

    if (textColor != context.FillColor())
    {
      context.SetFillColor(textColor);
    }

    bool setShadow = false;
    if (auto &shadows = lineStyle.GetTextShadow(); !shadows.IsNone())
    {
      const auto &zoomFactor = lineStyle.UsedZoomForLength();
      auto &shadow = shadows[0];
      context.SetDropShadow({LayoutSize(LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)),
                                        LayoutUnit(shadow.Location.y().ResolveZoom(zoomFactor))),
                             shadow.Blur.ResolveZoom(zoomFactor),
                             lineStyle.ColorWithColorFilter(shadow.Color), ShadowRadiusMode::Default});
      setShadow = true;
    }

    auto visualRect = _lineBox.EllipsisVisualRect();
    auto textOrigin = visualRect.Location();
    // Inline layout isn't subpixel, so the ascent is whole.
    auto ascent = LayoutUnit(lineStyle.MetricsOfPrimaryFont().IntAscent());
    textOrigin.Move(_paintOffset.X(), _paintOffset.Y() + ascent);

    auto deviceScaleFactor = _lineBox.FormattingContextRoot().GetDocument().DeviceScaleFactor();
    if (lineStyle.GetWritingMode().IsHorizontal())
    {
      textOrigin.SetY(RoundToDevicePixel(LayoutUnit {textOrigin.Y()}, deviceScaleFactor));
    }
    else
    {
      textOrigin.SetX(RoundToDevicePixel(LayoutUnit {textOrigin.X()}, deviceScaleFactor));
    }

    context.DrawBidiText(lineStyle.GetFontCascade(), _lineBox.EllipsisText(), textOrigin);

    if (textColor != context.FillColor())
    {
      context.SetFillColor(textColor);
    }

    if (setShadow)
    {
      context.ClearDropShadow();
    }
  }

  void EllipsisBoxPainter::PaintSelection() noexcept
  {
    auto &context = _paintInfo.Context();
    auto &style = _lineBox.Style();

    auto textColor = style.VisitedDependentColorWithColorFilter(css::PropertyId::Color);
    auto backgroundColor = _selectionBackgroundColor;
    if (!backgroundColor.IsVisible())
    {
      return;
    }

    // If the text color ends up being the same as the selection background, invert the selection background.
    if (textColor == backgroundColor)
    {
      backgroundColor = backgroundColor.InvertedColorWithAlpha(1.0);
    }

    GraphicsContextStateSaver stateSaver {context};

    auto visualRect =
      LayoutRect {_lineBox.EllipsisVisualRect(inline_iterator::LineBox::AdjustedForSelection::Yes)};
    visualRect.Move(_paintOffset.X(), _paintOffset.Y());

    auto ellipsisText = _lineBox.EllipsisText();
    constexpr bool canUseSimplifiedTextMeasuring = false;
    style.GetFontCascade().AdjustSelectionRectForText(canUseSimplifiedTextMeasuring, ellipsisText,
                                                      visualRect);
    context.FillRect(
      SnapRectToDevicePixelsWithWritingDirection(
        visualRect, _lineBox.FormattingContextRoot().GetDocument().DeviceScaleFactor(), ellipsisText.Ltr()),
      backgroundColor);
  }
}
