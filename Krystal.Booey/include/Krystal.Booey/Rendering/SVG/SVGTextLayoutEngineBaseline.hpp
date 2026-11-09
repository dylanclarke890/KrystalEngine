/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

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
