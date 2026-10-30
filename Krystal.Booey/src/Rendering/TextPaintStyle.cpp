#include "Krystal.Booey/Rendering/TextPaintStyle.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorLuminance.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
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
        // FIXME: The system appearance (Settings::useSystemAppearance), once there's a setting.
        paintStyle.FillColor = RenderTheme::Singleton().DefaultButtonTextColor(style::ColorOptions::None);
        return paintStyle;
      }
    }

    if (lineStyle.InsideSubmitButton())
    {
      paintStyle.FillColor = RenderTheme::Singleton().SubmitButtonTextColor(renderer);
      return paintStyle;
    }

    paintStyle.FillColor = lineStyle.VisitedDependentColorWithColorFilter(
      css::PropertyId::WebkitTextFillColor, paintInfo.PaintBehavior);

    bool forceBackgroundToWhite = false;
    if (renderer.GetDocument().Printing())
    {
      if (lineStyle.GetPrintColorAdjust() == PrintColorAdjust::Economy)
      {
        forceBackgroundToWhite = true;
      }

      // FIXME: The setting to print backgrounds (shouldPrintBackgrounds) comes with printing.

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

    paintStyle.StrokeColor = lineStyle.ColorByApplyingColorFilter(lineStyle.UsedStrokeColor());

    // Make the text stroke color legible against a white background
    if (forceBackgroundToWhite)
    {
      paintStyle.StrokeColor = AdjustColorForVisibilityOnBackground(paintStyle.StrokeColor, Color::white);
    }

    paintStyle.EmphasisMarkColor =
      lineStyle.VisitedDependentColorWithColorFilter(css::PropertyId::TextEmphasisColor);

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
