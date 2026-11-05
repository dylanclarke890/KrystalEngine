#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class FontMetrics;
  class RenderStyle;
  struct FontSizeAdjust;
  struct SettingsValues;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  enum class MinimumFontSizeRule : uint8
  {
    None,
    Absolute,
    AbsoluteAndRelative
  };

  struct ComputedFontSize
  {
    float Size {0.0f};
    float UsedZoomFactor {1.0f};
  };

  KRYS_NODISCARD float ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize,
                                                         float zoomFactor, MinimumFontSizeRule,
                                                         const SettingsValues &) noexcept;
  KRYS_NODISCARD ComputedFontSize ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize,
                                                                    bool useSVGZoomRules, const RenderStyle *,
                                                                    const dom::Document &) noexcept;
  KRYS_NODISCARD float ComputedFontSizeFromSpecifiedSizeForSVGInlineText(float specifiedSize,
                                                                         bool isAbsoluteSize,
                                                                         float zoomFactor,
                                                                         const dom::Document &) noexcept;
  KRYS_NODISCARD float AdjustedFontSize(float size, const boo::FontSizeAdjust &,
                                        const FontMetrics &) noexcept;

  // Given a CSS keyword id in the range (CSSValueXxSmall to CSSValueXxxLarge), this function will return
  // the correct font size scaled relative to the user's default (medium).
  KRYS_NODISCARD float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize,
                                          const SettingsValues &, bool inQuirksMode = false) noexcept;
  KRYS_NODISCARD float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize,
                                          const dom::Document &) noexcept;

  // Given a font size in pixel, this function will return legacy font size between 1 and 7.
  KRYS_NODISCARD int32 LegacyFontSizeForPixelSize(int32 pixelFontSize, bool shouldUseFixedDefaultSize,
                                                  const dom::Document &) noexcept;
}
