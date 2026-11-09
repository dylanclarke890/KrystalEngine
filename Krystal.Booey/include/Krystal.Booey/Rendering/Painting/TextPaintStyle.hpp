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

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPaintOrder.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class RenderStyle;
  class RenderText;
  struct PaintInfo;

  /// @brief The colours and stroke text is painted with.
  struct TextPaintStyle
  {
    Color FillColor;
    Color StrokeColor;
    Color EmphasisMarkColor;
    float StrokeWidth {0};
    bool HasExplicitlySetFillColor {false};
    style::SVGPaintOrder PaintOrder {css::keywords::Normal {}};
    LineJoin Join {LineJoin::Miter};
    LineCap Cap {LineCap::Butt};
    float MiterLimit {DefaultMiterLimit};

    TextPaintStyle() noexcept = default;
    explicit TextPaintStyle(const Color &color) noexcept;

    KRYS_NODISCARD bool operator==(const TextPaintStyle &other) const noexcept;
  };

  /// @brief Whether @p textColor reads against @p backgroundColor (a contrast ratio of 4.5:1 or more).
  KRYS_NODISCARD bool TextColorIsLegibleAgainstBackgroundColor(const Color &textColor,
                                                               const Color &backgroundColor) noexcept;
  /// @brief How @p renderer's text is painted on a line styled @p lineStyle.
  KRYS_NODISCARD TextPaintStyle ComputeTextPaintStyle(const RenderText &renderer,
                                                      const RenderStyle &lineStyle,
                                                      const PaintInfo &paintInfo) noexcept;
  /// @brief How the selected part of @p renderer's text is painted: @p textPaintStyle with the selection's
  /// colours, stroke and (into @p selectionShadow) shadow.
  KRYS_NODISCARD TextPaintStyle ComputeTextSelectionPaintStyle(const TextPaintStyle &textPaintStyle,
                                                               const RenderText &renderer,
                                                               const RenderStyle &lineStyle,
                                                               const PaintInfo &paintInfo,
                                                               style::TextShadows &selectionShadow) noexcept;

  enum FillColorType
  {
    UseNormalFillColor,
    UseEmphasisMarkColor
  };
  /// @brief Sets @p context's text drawing mode, fill and stroke to paint with @p paintStyle.
  void UpdateGraphicsContext(GraphicsContext &context, const TextPaintStyle &paintStyle,
                             FillColorType fillColorType = UseNormalFillColor) noexcept;
}
