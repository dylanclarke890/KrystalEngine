/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Painting/BorderEdge.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <cmath>

namespace krys::boo
{
  BorderEdge::BorderEdge(float edgeWidth, boo::Color edgeColor, BorderStyle edgeStyle, bool edgeIsTransparent,
                         bool edgeIsPresent, float devicePixelRatio) noexcept
      : _color(edgeColor), _width(edgeWidth), _devicePixelRatio(devicePixelRatio), _style(edgeStyle),
        _isTransparent(edgeIsTransparent), _isPresent(edgeIsPresent)
  {
    if (edgeStyle == BorderStyle::Double && edgeWidth < BorderWidthInDevicePixel(3))
    {
      _style = BorderStyle::Solid;
    }
    _flooredToDevicePixelWidth = std::floorf(edgeWidth * devicePixelRatio) / devicePixelRatio;
  }

  BorderEdges GetBorderEdges(const RenderStyle &style, float deviceScaleFactor, RectEdges<bool> closedEdges,
                             LayoutSize inflation, bool setColorsToBlack) noexcept
  {
    auto constructBorderEdge = [&](const RenderStyle &style, style::LineWidth width, float inflation,
                                   css::PropertyId borderColorProperty, BorderStyle borderStyle,
                                   bool isTransparent, bool isPresent)
    {
      auto color = setColorsToBlack ? boo::Color(boo::Color::black)
                                    : style.ColorResolvingCurrentColor(borderColorProperty);
      auto evaluatedWidth = style::Evaluate<float>(width, style::ZoomNeeded {});
      auto inflatedWidth = evaluatedWidth ? evaluatedWidth + inflation : evaluatedWidth;
      return BorderEdge(inflatedWidth, color, borderStyle, !setColorsToBlack && isTransparent, isPresent,
                        deviceScaleFactor);
    };

    return {constructBorderEdge(style, style.BorderTopWidth(), inflation.Height().ToFloat(),
                                css::PropertyId::BorderTopColor, style.BorderTopStyle(),
                                style.BorderTopIsTransparent(), closedEdges.Top()),
            constructBorderEdge(style, style.BorderRightWidth(), inflation.Width().ToFloat(),
                                css::PropertyId::BorderRightColor, style.BorderRightStyle(),
                                style.BorderRightIsTransparent(), closedEdges.Right()),
            constructBorderEdge(style, style.BorderBottomWidth(), inflation.Height().ToFloat(),
                                css::PropertyId::BorderBottomColor, style.BorderBottomStyle(),
                                style.BorderBottomIsTransparent(), closedEdges.Bottom()),
            constructBorderEdge(style, style.BorderLeftWidth(), inflation.Width().ToFloat(),
                                css::PropertyId::BorderLeftColor, style.BorderLeftStyle(),
                                style.BorderLeftIsTransparent(), closedEdges.Left())};
  }

  BorderEdges GetBorderEdgesForOutline(const RenderStyle &style, BorderStyle borderStyle,
                                       float deviceScaleFactor) noexcept
  {
    auto color = style.ColorResolvingCurrentColor(css::PropertyId::OutlineColor);
    auto isTransparent = color.IsValid() && !color.IsVisible();
    auto size = style::Evaluate<float>(style.OutlineWidth(), style::ZoomNeeded {});
    return {BorderEdge {size, color, borderStyle, isTransparent, true, deviceScaleFactor},
            BorderEdge {size, color, borderStyle, isTransparent, true, deviceScaleFactor},
            BorderEdge {size, color, borderStyle, isTransparent, true, deviceScaleFactor},
            BorderEdge {size, color, borderStyle, isTransparent, true, deviceScaleFactor}};
  }

  bool BorderEdge::ObscuresBackgroundEdge(float scale) const noexcept
  {
    if (!_isPresent || _isTransparent || (_width * scale) < BorderWidthInDevicePixel(2) || !_color.IsOpaque()
        || _style == BorderStyle::Hidden)
    {
      return false;
    }

    if (_style == BorderStyle::Dotted || _style == BorderStyle::Dashed)
    {
      return false;
    }

    if (_style == BorderStyle::Double)
    {
      return _width
             >= scale * BorderWidthInDevicePixel(5); // The outer band needs to be >= 2px wide at unit scale.
    }

    return true;
  }

  bool BorderEdge::ObscuresBackground() const noexcept
  {
    if (!_isPresent || _isTransparent || !_color.IsOpaque() || _style == BorderStyle::Hidden)
    {
      return false;
    }

    if (_style == BorderStyle::Dotted || _style == BorderStyle::Dashed || _style == BorderStyle::Double)
    {
      return false;
    }

    return true;
  }

  void BorderEdge::GetDoubleBorderStripeWidths(LayoutUnit &outerWidth, LayoutUnit &innerWidth) const noexcept
  {
    LayoutUnit fullWidth {WidthForPainting()};
    innerWidth = CeilToDevicePixel(fullWidth * 2 / 3, _devicePixelRatio);
    outerWidth = FloorToDevicePixel(fullWidth / 3, _devicePixelRatio);
  }
}
