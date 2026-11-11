/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/Painting/TextPainter.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Rendering/Painting/TextBoxPainter.hpp"
#include "Krystal.Booey/Rendering/RenderCombineText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  ShadowApplier::ShadowApplier(const RenderStyle &renderStyle, GraphicsContext &context,
                               const style::TextShadow *shadow, const FloatRect &textRect,
                               bool isLastShadowIteration, bool lastShadowIterationShouldDrawText,
                               bool opaque, bool ignoreWritingMode) noexcept
      : _context {context}, _shadow {shadow},
        _onlyDrawsShadow {!isLastShadowIteration || !lastShadowIterationShouldDrawText},
        _avoidDrawingShadow {ShadowIsCompletelyCoveredByText(opaque)},
        _nothingToDraw {shadow && _avoidDrawingShadow && _onlyDrawsShadow}, _didSaveContext {false}
  {
    if (!shadow || _nothingToDraw)
    {
      _shadow = nullptr;
      return;
    }

    const auto &zoomFactor = renderStyle.UsedZoomForLength();
    auto shadowOffset = TextBoxPainter::RotateShadowOffset(
      shadow->Location, ignoreWritingMode ? WritingMode() : renderStyle.GetWritingMode(), zoomFactor);
    auto shadowRadius = shadow->Blur.ResolveZoom(zoomFactor);
    auto shadowColor = renderStyle.ColorResolvingCurrentColor(shadow->Color);

    // When drawing shadows, we usually clip the context to the area the shadow will reside, and then
    // draw the text itself outside the clipped area (so only the shadow shows up). However, we can
    // often draw the *last* shadow and the text itself in a single call.
    if (_onlyDrawsShadow)
    {
      FloatRect shadowRect(textRect);
      shadowRect.Inflate(style::PaintingExtent(*shadow, zoomFactor) + 3 * textRect.Height());
      shadowRect.Move(shadowOffset);
      context.Save();
      context.Clip(shadowRect);

      _didSaveContext = true;
      _extraOffset =
        FloatSize(0, 2 * shadowRect.Height() + std::max(0.0f, shadowOffset.Height()) + shadowRadius);
      shadowOffset -= _extraOffset;
    }

    if (!_avoidDrawingShadow)
    {
      context.SetDropShadow({shadowOffset, shadowRadius, shadowColor});
    }
  }

  bool ShadowApplier::ShadowIsCompletelyCoveredByText(bool textIsOpaque) const noexcept
  {
    return textIsOpaque && _shadow && _shadow->Location.x().IsZero() && _shadow->Location.y().IsZero()
           && _shadow->Blur.IsZero();
  }

  ShadowApplier::~ShadowApplier() noexcept
  {
    if (!_shadow)
    {
      return;
    }
    if (_onlyDrawsShadow)
    {
      _context.Restore();
    }
    else if (!_avoidDrawingShadow)
    {
      _context.ClearDropShadow();
    }
  }

  TextPainter::TextPainter(GraphicsContext &context, const FontCascade &font, const RenderStyle &renderStyle,
                           const TextPaintStyle &textPaintStyle, const style::TextShadows &shadow,
                           const StringAtom &emphasisMark, float emphasisMarkOffset,
                           const RenderCombineText *combinedText) noexcept
      : _context(context), _font(font), _renderStyle(renderStyle), _style(textPaintStyle),
        _emphasisMark(emphasisMark), _shadow(shadow), _emphasisMarkOffset(emphasisMarkOffset),
        _combinedText(combinedText)
  {
  }

  void TextPainter::PaintTextOrEmphasisMarks(const FontCascade &font, const TextRun &textRun,
                                             const StringAtom &emphasisMark, float emphasisMarkOffset,
                                             const FloatPoint &textOrigin, uint32 startOffset,
                                             uint32 endOffset) noexcept
  {
    krys_debug_assert(startOffset < endOffset);

    if (_context.DetectingContentfulPaint())
    {
      if (!std::ranges::all_of(textRun.Text(), [](char8 c) { return text::IsASCIIWhitespace(c); }))
      {
        _context.SetContentfulPaintDetected();
      }
      return;
    }

    if (!emphasisMark.View().empty())
    {
      _context.DrawEmphasisMarks(font, textRun, emphasisMark, textOrigin + FloatSize(0, emphasisMarkOffset),
                                 startOffset, endOffset);
    }
    else
    {
      _context.DrawText(font, textRun, textOrigin, startOffset, endOffset);
    }
  }

  void TextPainter::PaintTextWithShadows(const style::TextShadows *shadows, const FontCascade &font,
                                         const TextRun &textRun, const FloatRect &boxRect,
                                         const FloatPoint &textOrigin, uint32 startOffset, uint32 endOffset,
                                         const StringAtom &emphasisMark, float emphasisMarkOffset,
                                         bool stroked) noexcept
  {
    if (!shadows || shadows->IsNone())
    {
      PaintTextOrEmphasisMarks(font, textRun, emphasisMark, emphasisMarkOffset, textOrigin, startOffset,
                               endOffset);
      return;
    }

    Color fillColor = _context.FillColor();
    bool opaque = fillColor.IsOpaque();
    bool lastShadowIterationShouldDrawText = !stroked && opaque;
    if (!opaque)
    {
      _context.SetFillColor(Color::black);
    }
    for (const auto &shadow : *shadows)
    {
      ShadowApplier shadowApplier(_renderStyle, _context, &shadow, boxRect, &shadow == &shadows->last(),
                                  lastShadowIterationShouldDrawText, opaque, _combinedText != nullptr);
      if (!shadowApplier.NothingToDraw())
      {
        PaintTextOrEmphasisMarks(font, textRun, emphasisMark, emphasisMarkOffset,
                                 textOrigin + shadowApplier.ExtraOffset(), startOffset, endOffset);
      }
    }

    if (!lastShadowIterationShouldDrawText)
    {
      if (!opaque)
      {
        _context.SetFillColor(fillColor);
      }
      PaintTextOrEmphasisMarks(font, textRun, emphasisMark, emphasisMarkOffset, textOrigin, startOffset,
                               endOffset);
    }
  }

  void TextPainter::PaintTextAndEmphasisMarksIfNeeded(const TextRun &textRun, const FloatRect &boxRect,
                                                      const FloatPoint &textOrigin, uint32 startOffset,
                                                      uint32 endOffset, const TextPaintStyle &paintStyle,
                                                      const style::TextShadows &shadow) noexcept
  {
    // Combined text too wide for an em is compressed horizontally about its origin.
    auto compression = _combinedText ? _combinedText->HorizontalScale() : 1.f;
    GraphicsContextStateSaver compressionSaver(_context, false);
    if (auto scale = compression; scale != 1.f)
    {
      compressionSaver.Save();
      _context.Translate(textOrigin.X(), 0);
      _context.Scale(FloatSize(scale, 1));
      _context.Translate(-textOrigin.X(), 0);
    }

    if (paintStyle.PaintOrder.IsNormal())
    {
      // FIXME: Truncate right-to-left text correctly.
      PaintTextWithShadows(&shadow, _font, textRun, boxRect, textOrigin, startOffset, endOffset,
                           StringAtom::Null(), 0, paintStyle.StrokeWidth > 0);
    }
    else
    {
      auto textDrawingMode = _context.GetTextDrawingMode();
      auto shadowToUse = &shadow;

      for (auto paintType : paintStyle.PaintOrder)
      {
        switch (paintType)
        {
          case style::PaintType::Fill:
          {
            auto textDrawingModeWithoutStroke = textDrawingMode;
            textDrawingModeWithoutStroke.Remove(TextDrawingMode::Stroke);
            _context.SetTextDrawingMode(textDrawingModeWithoutStroke);
            PaintTextWithShadows(shadowToUse, _font, textRun, boxRect, textOrigin, startOffset, endOffset,
                                 StringAtom::Null(), 0, false);
            shadowToUse = nullptr;
            _context.SetTextDrawingMode(textDrawingMode);
            break;
          }
          case style::PaintType::Stroke:
          {
            auto textDrawingModeWithoutFill = textDrawingMode;
            textDrawingModeWithoutFill.Remove(TextDrawingMode::Fill);
            _context.SetTextDrawingMode(textDrawingModeWithoutFill);
            PaintTextWithShadows(shadowToUse, _font, textRun, boxRect, textOrigin, startOffset, endOffset,
                                 StringAtom::Null(), 0, paintStyle.StrokeWidth > 0);
            shadowToUse = nullptr;
            _context.SetTextDrawingMode(textDrawingMode);
            break;
          }
          case style::PaintType::Markers: continue;
        }
      }
    }

    if (compression != 1.f)
    {
      compressionSaver.Restore();
    }

    if (_emphasisMark.View().empty())
    {
      return;
    }

    UpdateGraphicsContext(_context, paintStyle, UseEmphasisMarkColor);

    // Combined text takes one emphasis mark, over the middle of its box, drawn upright.
    static const TextRun objectReplacementCharacterTextRun {utf8_stringview {u8"\uFFFC"}};
    const auto &emphasisMarkTextRun = _combinedText ? objectReplacementCharacterTextRun : textRun;
    auto emphasisMarkTextOrigin = textOrigin;
    if (_combinedText)
    {
      auto boxOrigin = boxRect.Location();
      emphasisMarkTextOrigin = {boxOrigin.X() + boxRect.Width() / 2,
                                boxOrigin.Y() + static_cast<float>(_font.MetricsOfPrimaryFont().IntAscent())};
      _context.ConcatCTM(Rotation(boxRect, RotationDirection::Clockwise));
    }

    // FIXME: Truncate right-to-left text correctly.
    PaintTextWithShadows(&shadow, _combinedText ? _combinedText->OriginalFont() : _font, emphasisMarkTextRun,
                         boxRect, emphasisMarkTextOrigin, startOffset, endOffset, _emphasisMark,
                         _emphasisMarkOffset, paintStyle.StrokeWidth > 0);

    if (_combinedText)
    {
      _context.ConcatCTM(Rotation(boxRect, RotationDirection::Counterclockwise));
    }
  }

  void TextPainter::PaintRange(const TextRun &textRun, const FloatRect &boxRect, const FloatPoint &textOrigin,
                               uint32 start, uint32 end) noexcept
  {
    krys_debug_assert(start < end);
    PaintTextAndEmphasisMarksIfNeeded(textRun, boxRect, textOrigin, start, end, _style, _shadow);
  }
}
