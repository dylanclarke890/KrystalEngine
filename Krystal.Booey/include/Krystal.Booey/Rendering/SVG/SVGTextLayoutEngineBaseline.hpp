#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderElement;
  class RenderSVGInlineText;
  class RenderStyle;
  class SVGTextMetrics;

  /// @brief Helper used by SVGTextLayoutEngine to handle 'alignment-baseline' / 'dominant-baseline' and
  /// 'baseline-shift'.
  class SVGTextLayoutEngineBaseline
  {
    KRYS_NON_COPYABLE(SVGTextLayoutEngineBaseline);

  private:
    const FontCascade &_font;

  public:
    explicit SVGTextLayoutEngineBaseline(const FontCascade &) noexcept;

    KRYS_NODISCARD float CalculateBaselineShift(const RenderStyle &) const noexcept;
    KRYS_NODISCARD float
      CalculateAlignmentBaselineShift(bool isVerticalText,
                                      const RenderSVGInlineText &textRenderer) const noexcept;
    KRYS_NODISCARD float CalculateGlyphOrientationAngle(bool isVerticalText, const RenderStyle &,
                                                        char32 character) const noexcept;
    KRYS_NODISCARD float CalculateGlyphAdvanceAndOrientation(bool isVerticalText, SVGTextMetrics &,
                                                             float angle, float &xOrientationShift,
                                                             float &yOrientationShift) const noexcept;

  private:
    KRYS_NODISCARD AlignmentBaseline DominantBaselineToAlignmentBaseline(
      bool isVerticalText, const RenderElement &textRenderer) const noexcept;
  };
}
