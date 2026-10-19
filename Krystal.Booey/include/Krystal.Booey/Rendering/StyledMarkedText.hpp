#pragma once

#include "Krystal.Booey/Rendering/MarkedText.hpp"
#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "Krystal.Booey/Rendering/TextPaintStyle.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"

namespace krys::boo
{
  class RenderStyle;
  class RenderText;
  struct PaintInfo;

  /// @brief A marked text with how it's painted.
  struct StyledMarkedText final : MarkedText
  {
    struct Style
    {
      Color BackgroundColor;
      TextPaintStyle TextStyles;
      TextDecorationPainter::Styles TextDecorationStyles;
      style::TextShadows TextShadow {css::keywords::None {}};
      float Alpha {1};
    };

    Style MarkedStyle;

    StyledMarkedText(const MarkedText &marker) noexcept : MarkedText {marker}
    {
    }

    StyledMarkedText(const MarkedText &marker, Style style) noexcept : MarkedText {marker}, MarkedStyle(style)
    {
    }

    /// @brief @p textsToSubdivide split so they don't partly overlap, each with the style of the frontmost
    /// marked text covering it.
    KRYS_NODISCARD static List<StyledMarkedText> SubdivideAndResolve(const List<MarkedText> &textsToSubdivide,
                                                                     const RenderText &renderer,
                                                                     bool isFirstLine,
                                                                     const PaintInfo &paintInfo);
    KRYS_NODISCARD static List<StyledMarkedText>
      CoalesceAdjacentWithEqualBackground(const List<StyledMarkedText> &markedTexts);
    KRYS_NODISCARD static List<StyledMarkedText>
      CoalesceAdjacentWithEqualForeground(const List<StyledMarkedText> &markedTexts);
    KRYS_NODISCARD static List<StyledMarkedText>
      CoalesceAdjacentWithEqualDecorations(const List<StyledMarkedText> &markedTexts);
    /// @brief How @p renderer's text is painted where it isn't marked.
    KRYS_NODISCARD static Style ComputeStyleForUnmarkedMarkedText(const RenderText &renderer,
                                                                  const RenderStyle &lineStyle,
                                                                  bool isFirstLine,
                                                                  const PaintInfo &paintInfo) noexcept;
  };
}
