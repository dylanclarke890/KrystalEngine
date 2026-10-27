#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutEngineBaseline.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <cmath>
#include <unicode/uchar.h>

namespace krys::boo
{
  SVGTextLayoutEngineBaseline::SVGTextLayoutEngineBaseline(const FontCascade &font) noexcept : _font(font)
  {
  }

  float SVGTextLayoutEngineBaseline::CalculateBaselineShift(const RenderStyle &style) const noexcept
  {
    return style.BaselineShift().SwitchOn(
      [](const css::keywords::Baseline &) -> float { return 0; },
      [&](const css::keywords::Sub &) -> float { return -_font.MetricsOfPrimaryFont().Height() / 2; },
      [&](const css::keywords::Super &) -> float { return _font.MetricsOfPrimaryFont().Height() / 2; },
      [&](const style::SVGBaselineShift::Length &length) -> float
      { return style::Evaluate<float>(length, _font.Size(), style::ZoomNeeded {}); });
  }

  AlignmentBaseline SVGTextLayoutEngineBaseline::DominantBaselineToAlignmentBaseline(
    bool isVerticalText, const RenderElement &textRenderer) const noexcept
  {
    krys_debug_assert(textRenderer.Parent());

    DominantBaseline baseline = textRenderer.Style().GetDominantBaseline();
    if (baseline == DominantBaseline::Auto)
    {
      if (isVerticalText)
      {
        baseline = DominantBaseline::Central;
      }
      else
      {
        baseline = DominantBaseline::Alphabetic;
      }
    }

    switch (baseline)
    {
      case DominantBaseline::UseScript:
        // FIXME: The dominant-baseline and the baseline-table components are set by determining the
        // predominant script of the character data content.
        return AlignmentBaseline::Alphabetic;
      case DominantBaseline::NoChange:
        return DominantBaselineToAlignmentBaseline(isVerticalText, *textRenderer.Parent());
      case DominantBaseline::ResetSize:
        return DominantBaselineToAlignmentBaseline(isVerticalText, *textRenderer.Parent());
      case DominantBaseline::Ideographic:    return AlignmentBaseline::Ideographic;
      case DominantBaseline::Alphabetic:     return AlignmentBaseline::Alphabetic;
      case DominantBaseline::Hanging:        return AlignmentBaseline::Hanging;
      case DominantBaseline::Mathematical:   return AlignmentBaseline::Mathematical;
      case DominantBaseline::Central:        return AlignmentBaseline::Central;
      case DominantBaseline::Middle:         return AlignmentBaseline::Middle;
      case DominantBaseline::TextAfterEdge:  return AlignmentBaseline::TextAfterEdge;
      case DominantBaseline::TextBeforeEdge: return AlignmentBaseline::TextBeforeEdge;
      default:                               krys_debug_assert(false); return AlignmentBaseline::Baseline;
    }
  }

  float SVGTextLayoutEngineBaseline::CalculateAlignmentBaselineShift(
    bool isVerticalText, const RenderSVGInlineText &textRenderer) const noexcept
  {
    auto *textRendererParent = textRenderer.Parent();
    krys_debug_assert(textRendererParent);

    AlignmentBaseline baseline = textRenderer.Style().GetAlignmentBaseline();
    if (baseline == AlignmentBaseline::Baseline)
    {
      baseline = DominantBaselineToAlignmentBaseline(isVerticalText, *textRendererParent);
      krys_debug_assert(baseline != AlignmentBaseline::Baseline);
    }

    const FontMetrics &fontMetrics = _font.MetricsOfPrimaryFont();
    float ascent = fontMetrics.Ascent();
    float descent = fontMetrics.Descent();

    // Note: http://wiki.apache.org/xmlgraphics-fop/LineLayout/AlignmentHandling
    switch (baseline)
    {
      case AlignmentBaseline::BeforeEdge:
      case AlignmentBaseline::TextBeforeEdge: return ascent;
      case AlignmentBaseline::Middle:         return fontMetrics.XHeight().value_or(0.f) / 2;
      case AlignmentBaseline::Central:        return (ascent - descent) / 2;
      case AlignmentBaseline::AfterEdge:
      case AlignmentBaseline::TextAfterEdge:
      case AlignmentBaseline::Ideographic:    return -descent;
      case AlignmentBaseline::Alphabetic:     return 0;
      case AlignmentBaseline::Hanging:        return ascent * 8 / 10.f;
      case AlignmentBaseline::Mathematical:   return ascent / 2;
      case AlignmentBaseline::Baseline:       krys_debug_assert(false); return 0;
    }

    krys_debug_assert(false);
    return 0;
  }

  float SVGTextLayoutEngineBaseline::CalculateGlyphOrientationAngle(bool isVerticalText,
                                                                    const RenderStyle &style,
                                                                    char32 character) const noexcept
  {
    if (isVerticalText)
    {
      return style::ValueRepresentation<style::SVGGlyphOrientationVertical> {}(
        style.GlyphOrientationVertical(),
        [&](const css::keywords::Auto &)
        {
          // Spec: Fullwidth ideographic and fullwidth Latin text will be set with a glyph-orientation of
          // 0-degrees. Text which is not fullwidth will be set with a glyph-orientation of 90-degrees.
          // FIXME: There's not an accurate way to tell if text is fullwidth by looking at a single character.
          switch (static_cast<UEastAsianWidth>(
            u_getIntPropertyValue(static_cast<UChar32>(character), UCHAR_EAST_ASIAN_WIDTH)))
          {
            case U_EA_NEUTRAL:
            case U_EA_HALFWIDTH:
            case U_EA_NARROW:    return 90.0f;
            case U_EA_AMBIGUOUS:
            case U_EA_FULLWIDTH:
            case U_EA_WIDE:      return 0.0f;
            default:             break;
          }
          krys_debug_assert(false);
          return 0.0f;
        },
        [](const style::Angle<> &angle) { return static_cast<float>(angle.Value); });
    }

    return style::ValueRepresentation<style::SVGGlyphOrientationHorizontal> {}(
      style.GlyphOrientationHorizontal(),
      [](const style::Angle<> &angle) { return static_cast<float>(angle.Value); });
  }

  namespace
  {
    KRYS_NODISCARD bool GlyphOrientationIsMultiplyOf180Degrees(float orientationAngle) noexcept
    {
      return !(std::fmod(orientationAngle, 180.f));
    }
  }

  float SVGTextLayoutEngineBaseline::CalculateGlyphAdvanceAndOrientation(
    bool isVerticalText, SVGTextMetrics &metrics, float angle, float &xOrientationShift,
    float &yOrientationShift) const noexcept
  {
    bool orientationIsMultiplyOf180Degrees = GlyphOrientationIsMultiplyOf180Degrees(angle);

    // The function is based on spec requirements:
    //
    // Spec: If the 'glyph-orientation-horizontal' results in an orientation angle that is not a multiple of
    // of 180 degrees, then the current text position is incremented according to the vertical metrics of the
    // glyph.
    //
    // Spec: If if the 'glyph-orientation-vertical' results in an orientation angle that is not a multiple of
    // 180 degrees, then the current text position is incremented according to the horizontal metrics of the
    // glyph.

    const FontMetrics &fontMetrics = _font.MetricsOfPrimaryFont();
    float ascent = fontMetrics.Ascent();
    float descent = fontMetrics.Descent();

    // Vertical orientation handling.
    if (isVerticalText)
    {
      float ascentMinusDescent = ascent - descent;
      if (!angle)
      {
        xOrientationShift = (ascentMinusDescent - metrics.Width()) / 2;
        yOrientationShift = ascent;
      }
      else if (angle == 180)
      {
        xOrientationShift = (ascentMinusDescent + metrics.Width()) / 2;
      }
      else if (angle == 270)
      {
        yOrientationShift = metrics.Width();
        xOrientationShift = ascentMinusDescent;
      }

      // Vertical advance calculation.
      if (angle && !orientationIsMultiplyOf180Degrees)
      {
        return metrics.Width();
      }

      return metrics.Height();
    }

    // Horizontal orientation handling.
    if (angle == 90)
    {
      yOrientationShift = -metrics.Width();
    }
    else if (angle == 180)
    {
      xOrientationShift = metrics.Width();
      yOrientationShift = -ascent;
    }
    else if (angle == 270)
    {
      xOrientationShift = metrics.Width();
    }

    // Horizontal advance calculation.
    if (angle && !orientationIsMultiplyOf180Degrees)
    {
      return metrics.Height();
    }

    return metrics.Width();
  }
}
