/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Rendering/Painting/MarkedText.hpp"
#include "Krystal.Booey/Rendering/Painting/TextDecorationPainter.hpp"
#include "Krystal.Booey/Rendering/Painting/TextPaintStyle.hpp"
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
