/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleBase+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties.hpp"

#include "Krystal.Booey/Animation/Timelines/ViewTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBackdropFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFlexibleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFontData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGridData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGridItemData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"
#include "Krystal.Booey/Style/StyleAppearance.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontPalette.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontStyle.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantAlternates.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantEastAsian.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantLigatures.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantNumeric.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariationSettings.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontWeight.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontWidth.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAutospace.hpp"
#include "Krystal.Booey/Style/Values/Text/TextSpacingTrim.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Text/UnicodeBidi.hpp"

namespace krys::boo::style
{
  constexpr SVGGlyphOrientationHorizontal
    ComputedStyleProperties::InitialGlyphOrientationHorizontal() noexcept
  {
    return SVGGlyphOrientationHorizontal::Degrees0;
  }

  inline boo::Color ComputedStyleProperties::InitialColor() noexcept
  {
    return boo::Color::black;
  }

  constexpr LineWidth ComputedStyleProperties::InitialBorderBottomWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr LineWidth ComputedStyleProperties::InitialBorderLeftWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr LineWidth ComputedStyleProperties::InitialBorderRightWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr LineWidth ComputedStyleProperties::InitialBorderTopWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr LineWidth ComputedStyleProperties::InitialColumnRuleWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr LineWidth ComputedStyleProperties::InitialOutlineWidth() noexcept
  {
    return LineWidth {3.0f};
  }

  constexpr WebkitLineBoxContain ComputedStyleProperties::InitialLineBoxContain() noexcept
  {
    return {WebkitLineBoxContainValue::Block, WebkitLineBoxContainValue::Inline,
            WebkitLineBoxContainValue::Replaced};
  }

  constexpr TextEmphasisPosition ComputedStyleProperties::InitialTextEmphasisPosition() noexcept
  {
    return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
  }

  constexpr PositionVisibility ComputedStyleProperties::InitialPositionVisibility() noexcept
  {
    return PositionVisibilityValue::AnchorsVisible;
  }
}
