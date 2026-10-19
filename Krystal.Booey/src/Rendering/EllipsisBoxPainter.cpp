#include "Krystal.Booey/Rendering/EllipsisBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  EllipsisBoxPainter::EllipsisBoxPainter(const inline_iterator::LineBox &lineBox, PaintInfo &paintInfo,
                                         const LayoutPoint &paintOffset) noexcept
      : _lineBox(lineBox), _paintInfo(paintInfo), _paintOffset(paintOffset)
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

    // FIXME: A selected ellipsis's background and colour come with selection.

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

    // FIXME: The ellipsis is drawn as one run (drawBidiText waits for a bidi resolver over text runs).
    context.DrawText(lineStyle.GetFontCascade(), _lineBox.EllipsisText(), textOrigin);

    if (textColor != context.FillColor())
    {
      context.SetFillColor(textColor);
    }

    if (setShadow)
    {
      context.ClearDropShadow();
    }
  }
}
