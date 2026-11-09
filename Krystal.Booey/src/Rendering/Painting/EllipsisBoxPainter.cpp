/*
 * Copyright (C) 2022-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Painting/EllipsisBoxPainter.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
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
    auto textColor = lineStyle.ColorResolvingCurrentColor(css::PropertyId::Color);

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
                             lineStyle.ColorResolvingCurrentColor(shadow.Color), ShadowRadiusMode::Default});
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

    auto textColor = style.ColorResolvingCurrentColor(css::PropertyId::Color);
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
