/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Painting/TextPaintStyle.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorLuminance.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"

namespace krys::boo
{
  TextPaintStyle::TextPaintStyle(const Color &color) noexcept : FillColor(color), StrokeColor(color)
  {
  }

  bool TextPaintStyle::operator==(const TextPaintStyle &other) const noexcept
  {
    return FillColor == other.FillColor && StrokeColor == other.StrokeColor
           && EmphasisMarkColor == other.EmphasisMarkColor && StrokeWidth == other.StrokeWidth
           && PaintOrder == other.PaintOrder && Join == other.Join && Cap == other.Cap
           && MiterLimit == other.MiterLimit;
  }

  bool TextColorIsLegibleAgainstBackgroundColor(const Color &textColor, const Color &backgroundColor) noexcept
  {
    // Uses the WCAG 2.0 definition of legibility: a contrast ratio of 4.5:1 or greater.
    // https://www.w3.org/TR/WCAG20/#visual-audio-contrast-contrast
    return ContrastRatio(textColor, backgroundColor) >= 4.5;
  }

  KRYS_NODISCARD static Color AdjustColorForVisibilityOnBackground(const Color &textColor,
                                                                   const Color &backgroundColor) noexcept
  {
    if (TextColorIsLegibleAgainstBackgroundColor(textColor, backgroundColor))
    {
      return textColor;
    }

    if (textColor.Luminance() > 0.5)
    {
      return textColor.Darkened();
    }
    return textColor.Lightened();
  }

  TextPaintStyle ComputeTextSelectionPaintStyle(const TextPaintStyle &textPaintStyle,
                                                const RenderText &renderer, const RenderStyle &lineStyle,
                                                const PaintInfo &paintInfo,
                                                style::TextShadows &selectionShadow) noexcept
  {
    TextPaintStyle selectionPaintStyle = textPaintStyle;

    Color foreground =
      paintInfo.ForceTextColor() ? paintInfo.ForcedTextColor() : renderer.SelectionForegroundColor();
    if (foreground.IsValid() && foreground != selectionPaintStyle.FillColor)
    {
      selectionPaintStyle.FillColor = foreground;
    }

    Color emphasisMarkForeground =
      paintInfo.ForceTextColor() ? paintInfo.ForcedTextColor() : renderer.SelectionEmphasisMarkColor();
    if (emphasisMarkForeground.IsValid() && emphasisMarkForeground != selectionPaintStyle.EmphasisMarkColor)
    {
      selectionPaintStyle.EmphasisMarkColor = emphasisMarkForeground;
    }

    if (auto pseudoStyle = renderer.SelectionPseudoStyle())
    {
      selectionPaintStyle.HasExplicitlySetFillColor = pseudoStyle->HasExplicitlySetColor();
      selectionShadow = paintInfo.ForceTextColor() ? style::TextShadows {css::keywords::None {}}
                                                   : pseudoStyle->GetTextShadow();
      auto viewportSize = renderer.View().FrameView().Size();
      float strokeWidth = pseudoStyle->UsedStrokeWidth(viewportSize);
      if (strokeWidth != selectionPaintStyle.StrokeWidth)
      {
        selectionPaintStyle.StrokeWidth = strokeWidth;
      }

      Color stroke =
        paintInfo.ForceTextColor() ? paintInfo.ForcedTextColor() : pseudoStyle->UsedStrokeColor();
      if (stroke != selectionPaintStyle.StrokeColor)
      {
        selectionPaintStyle.StrokeColor = stroke;
      }
    }
    else
    {
      selectionShadow =
        paintInfo.ForceTextColor() ? style::TextShadows {css::keywords::None {}} : lineStyle.GetTextShadow();
    }
    return selectionPaintStyle;
  }

  TextPaintStyle ComputeTextPaintStyle(const RenderText &renderer, const RenderStyle &lineStyle,
                                       const PaintInfo &paintInfo) noexcept
  {
    TextPaintStyle paintStyle;

    auto viewportSize = renderer.View().FrameView().Size();
    paintStyle.StrokeWidth = lineStyle.UsedStrokeWidth(viewportSize);
    paintStyle.PaintOrder = lineStyle.PaintOrder();
    paintStyle.Join = lineStyle.JoinStyle();
    paintStyle.Cap = lineStyle.CapStyle();
    paintStyle.MiterLimit = lineStyle.StrokeMiterLimit().Value.Value;

    if (paintInfo.ForceTextColor())
    {
      paintStyle.FillColor = paintInfo.ForcedTextColor();
      paintStyle.StrokeColor = paintInfo.ForcedTextColor();
      paintStyle.EmphasisMarkColor = paintInfo.ForcedTextColor();
      return paintStyle;
    }

    if (lineStyle.InsideDefaultButton())
    {
      if (renderer.GetDocument().IsFocusedAndActive())
      {
        auto options = renderer.GetDocument().GetSettings().UseSystemAppearance
                         ? style::ColorOptions::UseSystemAppearance
                         : style::ColorOptions::None;
        paintStyle.FillColor = RenderTheme::Singleton().DefaultButtonTextColor(options);
        return paintStyle;
      }
    }

    if (lineStyle.InsideSubmitButton())
    {
      paintStyle.FillColor = RenderTheme::Singleton().SubmitButtonTextColor(renderer);
      return paintStyle;
    }

    paintStyle.FillColor = lineStyle.ColorResolvingCurrentColor(css::PropertyId::Color);

    bool forceBackgroundToWhite = false;
    if (renderer.GetDocument().Printing())
    {
      if (lineStyle.GetPrintColorAdjust() == PrintColorAdjust::Economy)
      {
        forceBackgroundToWhite = true;
      }

      if (forceBackgroundToWhite)
      {
        if (style::HasAnyBackgroundClipText(renderer.Style().GetBackgroundLayers()))
        {
          paintStyle.FillColor = Color::black;
        }
      }
    }

    // Make the text fill color legible against a white background
    if (forceBackgroundToWhite)
    {
      paintStyle.FillColor = AdjustColorForVisibilityOnBackground(paintStyle.FillColor, Color::white);
    }

    paintStyle.StrokeColor = lineStyle.UsedStrokeColor();

    // Make the text stroke color legible against a white background
    if (forceBackgroundToWhite)
    {
      paintStyle.StrokeColor = AdjustColorForVisibilityOnBackground(paintStyle.StrokeColor, Color::white);
    }

    paintStyle.EmphasisMarkColor = lineStyle.ColorResolvingCurrentColor(css::PropertyId::TextEmphasisColor);

    // Make the text stroke color legible against a white background
    if (forceBackgroundToWhite)
    {
      paintStyle.EmphasisMarkColor =
        AdjustColorForVisibilityOnBackground(paintStyle.EmphasisMarkColor, Color::white);
    }

    return paintStyle;
  }

  void UpdateGraphicsContext(GraphicsContext &context, const TextPaintStyle &paintStyle,
                             FillColorType fillColorType) noexcept
  {
    TextDrawingModeFlags mode = context.GetTextDrawingMode();
    TextDrawingModeFlags newMode = mode;
    if (paintStyle.StrokeWidth > 0 && paintStyle.StrokeColor.IsVisible())
    {
      newMode.Add(TextDrawingMode::Stroke);
    }
    if (mode != newMode)
    {
      context.SetTextDrawingMode(newMode);
      mode = newMode;
    }

    Color fillColor =
      fillColorType == UseEmphasisMarkColor ? paintStyle.EmphasisMarkColor : paintStyle.FillColor;
    if (mode.Contains(TextDrawingMode::Fill) && (fillColor != context.FillColor()))
    {
      context.SetFillColor(fillColor);
    }

    if (mode.Contains(TextDrawingMode::Stroke))
    {
      if (paintStyle.StrokeColor != context.StrokeColor())
      {
        context.SetStrokeColor(paintStyle.StrokeColor);
      }
      if (paintStyle.StrokeWidth != context.StrokeThickness())
      {
        context.SetStrokeThickness(paintStyle.StrokeWidth);
      }
      context.SetLineJoin(paintStyle.Join);
      context.SetLineCap(paintStyle.Cap);
      if (paintStyle.Join == LineJoin::Miter)
      {
        context.SetMiterLimit(paintStyle.MiterLimit);
      }
    }
  }
}
