#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <span>

namespace krys::boo::style
{
  float ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize, float zoomFactor,
                                          MinimumFontSizeRule minimumSizeRule,
                                          const SettingsValues &settings) noexcept
  {
    // Text with a 0px font size should not be visible and therefore needs to be
    // exempt from minimum font size rules. Acid3 relies on this for pixel-perfect
    // rendering. This is also compatible with other browsers that have minimum
    // font size settings (e.g. Firefox).
    if (std::abs(specifiedSize) < std::numeric_limits<float>::epsilon())
    {
      return 0.0f;
    }

    // We support two types of minimum font size. The first is a hard override that applies to
    // all fonts. This is "minSize." The second type of minimum font size is a "smart minimum"
    // that is applied only when the Web page can't know what size it really asked for, e.g.,
    // when it uses logical sizes like "small" or expresses the font-size as a percentage of
    // the user's default font setting.

    // With the smart minimum, we never want to get smaller than the minimum font size to keep fonts readable.
    // However we always allow the page to set an explicit pixel size that is smaller,
    // since sites will mis-render otherwise (e.g., http://www.gamespot.com with a 9px minimum).

    if (minimumSizeRule == MinimumFontSizeRule::None)
    {
      return specifiedSize;
    }

    int32 minSize = settings.MinimumFontSize;
    int32 minLogicalSize = settings.MinimumLogicalFontSize;
    float zoomedSize = specifiedSize * zoomFactor;

    // Apply the hard minimum first. We only apply the hard minimum if after zooming we're still too small.
    zoomedSize = std::max(zoomedSize, static_cast<float>(minSize));

    // Now apply the smart minimum. This minimum is also only applied if we're still too small
    // after zooming. The font size must either be relative to the user default or the original size
    // must have been acceptable. In other words, we only apply the smart minimum whenever we're positive
    // doing so won't disrupt the layout.
    if (minimumSizeRule == MinimumFontSizeRule::AbsoluteAndRelative
        && (specifiedSize >= minLogicalSize || !isAbsoluteSize))
    {
      zoomedSize = std::max(zoomedSize, static_cast<float>(minLogicalSize));
    }

    // Also clamp to a reasonable maximum to prevent insane font sizes from causing crashes on various
    // platforms. (I'm looking at you, Windows.)
    return std::min(MaximumAllowedFontSize, zoomedSize);
  }

  ComputedFontSize ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize,
                                                     bool useSVGZoomRules, const RenderStyle *style,
                                                     const dom::Document &document) noexcept
  {
    float zoomFactor = 1.0f;
    if (!useSVGZoomRules)
    {
      zoomFactor = style->UsedZoom();
      // FIXME: Documents have no frame, so there's no text zoom factor (`-webkit-text-zoom` isn't supported).
    }
    return {ComputedFontSizeFromSpecifiedSize(specifiedSize, isAbsoluteSize, zoomFactor,
                                              useSVGZoomRules ? MinimumFontSizeRule::None
                                                              : MinimumFontSizeRule::AbsoluteAndRelative,
                                              document.GetSettings().Values),
            zoomFactor};
  }

  float ComputedFontSizeFromSpecifiedSizeForSVGInlineText(float specifiedSize, bool isAbsoluteSize,
                                                          float zoomFactor,
                                                          const dom::Document &document) noexcept
  {
    return ComputedFontSizeFromSpecifiedSize(specifiedSize, isAbsoluteSize, zoomFactor,
                                             MinimumFontSizeRule::Absolute, document.GetSettings().Values);
  }

  constexpr int32 fontSizeTableMax = 16;
  constexpr int32 fontSizeTableMin = 9;

  // WinIE/Nav4 table for font sizes. Designed to match the legacy font mapping system of HTML.
  static constexpr std::array quirksFontSizeTable {
    std::array {9, 9, 9, 9, 11, 14, 18, 28},   std::array {9, 9, 9, 10, 12, 15, 20, 31},
    std::array {9, 9, 9, 11, 13, 17, 22, 34},  std::array {9, 9, 10, 12, 14, 18, 24, 37},
    std::array {9, 9, 10, 13, 16, 20, 26, 40}, // fixed font default (13)
    std::array {9, 9, 11, 14, 17, 21, 28, 42}, std::array {9, 10, 12, 15, 17, 23, 30, 45},
    std::array {9, 10, 13, 16, 18, 24, 32, 48} // proportional font default (16)
  };
  // HTML       1      2      3      4      5      6      7
  // CSS  xxs   xs     s      m      l     xl     xxl
  //                          |
  //                      user pref

  // Strict mode table matches MacIE and Mozilla's settings exactly.
  static constexpr std::array strictFontSizeTable {
    std::array {9, 9, 9, 9, 11, 14, 18, 27},    std::array {9, 9, 9, 10, 12, 15, 20, 30},
    std::array {9, 9, 10, 11, 13, 17, 22, 33},  std::array {9, 9, 10, 12, 14, 18, 24, 36},
    std::array {9, 10, 12, 13, 16, 20, 26, 39}, // fixed font default (13)
    std::array {9, 10, 12, 14, 17, 21, 28, 42}, std::array {9, 10, 13, 15, 18, 23, 30, 45},
    std::array {9, 10, 13, 16, 18, 24, 32, 48} // proportional font default (16)
  };
  // HTML       1      2      3      4      5      6      7
  // CSS  xxs   xs     s      m      l     xl     xxl
  //                          |
  //                      user pref

  // For values outside the range of the table, we use Todd Fahrner's suggested scale
  // factors for each keyword value.
  static constexpr std::array fontSizeFactors {0.60f, 0.75f, 0.89f, 1.0f, 1.2f, 1.5f, 2.0f, 3.0f};

  float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize, const SettingsValues &settings,
                           bool inQuirksMode) noexcept
  {
    int32 mediumSize = shouldUseFixedDefaultSize ? settings.DefaultFixedFontSize : settings.DefaultFontSize;
    if (mediumSize >= fontSizeTableMin && mediumSize <= fontSizeTableMax)
    {
      // Look up the entry in the table.
      int32 row = mediumSize - fontSizeTableMin;
      int32 col = static_cast<int32>(keywordID) - static_cast<int32>(css::ValueId::XxSmall);
      return static_cast<float>(inQuirksMode ? quirksFontSizeTable[row][col] : strictFontSizeTable[row][col]);
    }

    // Value is outside the range of the table. Apply the scale factor instead.
    float minLogicalSize = std::max<float>(static_cast<float>(settings.MinimumLogicalFontSize), 1);
    return std::max(fontSizeFactors[keywordID - static_cast<uint32>(css::ValueId::XxSmall)]
                      * static_cast<float>(mediumSize),
                    minLogicalSize);
  }

  float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize,
                           const dom::Document &document) noexcept
  {
    return FontSizeForKeyword(keywordID, shouldUseFixedDefaultSize, document.GetSettings().Values,
                              document.InQuirksMode());
  }

  template <typename T, size_t Extent>
  KRYS_NODISCARD static int32 FindNearestLegacyFontSize(int32 pixelFontSize, std::span<const T, Extent> table,
                                                        int32 multiplier) noexcept
  {
    // Ignore table[0] because xx-small does not correspond to any legacy font size.
    for (size_t i = 1; i < table.size() - 1; ++i)
    {
      if (static_cast<T>(pixelFontSize * 2) < (table[i] + table[i + 1]) * static_cast<T>(multiplier))
      {
        return static_cast<int32>(i);
      }
    }
    return static_cast<int32>(table.size() - 1);
  }

  int32 LegacyFontSizeForPixelSize(int32 pixelFontSize, bool shouldUseFixedDefaultSize,
                                   const dom::Document &document) noexcept
  {
    bool quirksMode = document.InQuirksMode();
    const auto &settings = document.GetSettings().Values;
    int32 mediumSize = shouldUseFixedDefaultSize ? settings.DefaultFixedFontSize : settings.DefaultFontSize;
    if (mediumSize >= fontSizeTableMin && mediumSize <= fontSizeTableMax)
    {
      int32 row = mediumSize - fontSizeTableMin;
      return FindNearestLegacyFontSize(
        pixelFontSize, std::span {quirksMode ? quirksFontSizeTable[row] : strictFontSizeTable[row]}, 1);
    }

    return FindNearestLegacyFontSize(pixelFontSize, std::span {fontSizeFactors}, mediumSize);
  }

  KRYS_NODISCARD static float AdjustedFontSize(float size, float sizeAdjust, float metricValue) noexcept
  {
    krys_debug_assert(sizeAdjust > 0);
    if (!size)
    {
      return 0;
    }

    float aspectValue = metricValue / size;
    return size * (sizeAdjust / aspectValue);
  }

  float AdjustedFontSize(float size, const boo::FontSizeAdjust &sizeAdjust,
                         const FontMetrics &metrics) noexcept
  {
    // FIXME: The behavior for missing metrics has yet to be defined.
    // https://github.com/w3c/csswg-drafts/issues/6384
    switch (sizeAdjust.MetricType)
    {
      case boo::FontSizeAdjust::Metric::CapHeight:
        return metrics.CapHeight() ? AdjustedFontSize(size, *sizeAdjust.Value, *metrics.CapHeight()) : size;
      case boo::FontSizeAdjust::Metric::ChWidth:
        return metrics.ZeroWidth() ? AdjustedFontSize(size, *sizeAdjust.Value, *metrics.ZeroWidth()) : size;
      // FIXME: Are ic-height and ic-width the same? Gecko treats them the same.
      case boo::FontSizeAdjust::Metric::IcWidth:
      case boo::FontSizeAdjust::Metric::IcHeight:
        return metrics.IdeogramWidth() ? AdjustedFontSize(size, *sizeAdjust.Value, *metrics.IdeogramWidth())
                                       : size;
      case boo::FontSizeAdjust::Metric::ExHeight:
      default:
        return metrics.XHeight() ? AdjustedFontSize(size, *sizeAdjust.Value, *metrics.XHeight()) : size;
    }
  }
}
