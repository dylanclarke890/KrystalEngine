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

#include "Krystal.Web/Style/Computed/ComputedStyleBase+GettersInlines.hpp"
#include "Krystal.Web/Style/Computed/ComputedStyleProperties.hpp"

#include "Krystal.Web/Animation/Timelines/ViewTimeline.hpp"
#include "Krystal.Web/Animation/WebAnimationTypes.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCascadeDescription.hpp"
#include "Krystal.Web/Graphics/GraphicsTypes.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Web/Rendering/Style/PositionTryOrder.hpp"
#include "Krystal.Web/Rendering/Style/StyleBackdropFilterData.hpp"
#include "Krystal.Web/Rendering/Style/StyleBackgroundData.hpp"
#include "Krystal.Web/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Web/Rendering/Style/StyleFilterData.hpp"
#include "Krystal.Web/Rendering/Style/StyleFlexibleBoxData.hpp"
#include "Krystal.Web/Rendering/Style/StyleFontData.hpp"
#include "Krystal.Web/Rendering/Style/StyleGridData.hpp"
#include "Krystal.Web/Rendering/Style/StyleGridItemData.hpp"
#include "Krystal.Web/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Web/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Web/Rendering/Style/StyleMultiColData.hpp"
#include "Krystal.Web/Rendering/Style/StyleNonInheritedData.hpp"
#include "Krystal.Web/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Web/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Web/Rendering/Style/StyleSurroundData.hpp"
#include "Krystal.Web/Rendering/Style/StyleTransformData.hpp"
#include "Krystal.Web/Rendering/Style/SVGRenderStyle.hpp"
#include "Krystal.Web/Style/StyleAppearance.hpp"
#include "Krystal.Web/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontPalette.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontStyle.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantAlternates.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantEastAsian.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantLigatures.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantNumeric.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariationSettings.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontWeight.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontWidth.hpp"
#include "Krystal.Web/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Web/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Web/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Web/Style/Values/Text/TextAutospace.hpp"
#include "Krystal.Web/Style/Values/Text/TextSpacingTrim.hpp"
#include "Krystal.Web/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Web/Text/UnicodeBidi.hpp"

namespace krys::web::style
{
  // FIXME: Support generating properties that have their storage spread out

  inline Cursor ComputedStyleProperties::Cursor() const noexcept
  {
    return {_rareInheritedData->CursorImages, static_cast<web::CursorType>(_inheritedFlags.CursorType)};
  }

  inline ZIndex ComputedStyleProperties::SpecifiedZIndex() const noexcept
  {
    return _nonInheritedData->BoxData->SpecifiedZIndex();
  }

  // FIXME: Support writing mode properties.

  inline text::TextDirection ComputedStyleProperties::ComputedDirection() const noexcept
  {
    return GetWritingMode().ComputedTextDirection();
  }

  inline StyleWritingMode ComputedStyleProperties::ComputedWritingMode() const noexcept
  {
    return GetWritingMode().ComputedWritingMode();
  }

  inline TextOrientation ComputedStyleProperties::ComputedTextOrientation() const noexcept
  {
    return GetWritingMode().ComputedTextOrientation();
  }

  // FIXME: Support properties where the getter returns a different value than the setter checks for equality
  // or rename these to be used*() and generate the real getters.

  inline LineWidth ComputedStyleProperties::BorderBottomWidth() const noexcept
  {
    return Border().BorderBottomWidth();
  }

  inline LineWidth ComputedStyleProperties::BorderLeftWidth() const noexcept
  {
    return Border().BorderLeftWidth();
  }

  inline LineWidth ComputedStyleProperties::BorderRightWidth() const noexcept
  {
    return Border().BorderRightWidth();
  }

  inline LineWidth ComputedStyleProperties::BorderTopWidth() const noexcept
  {
    return Border().BorderTopWidth();
  }

  inline LineWidth ComputedStyleProperties::ColumnRuleWidth() const noexcept
  {
    return _nonInheritedData->MiscData->MultiCol->ColumnRuleWidth();
  }

  // FIXME: Support font properties.

  inline float ComputedStyleProperties::SpecifiedFontSize() const noexcept
  {
    return GetFontDescription().SpecifiedSize();
  }

  inline FontFamilies ComputedStyleProperties::FontFamily() const noexcept
  {
    return {List<CSSOMStringAtom> {GetFontDescription().Families()}, GetFontDescription().IsSpecifiedFont()};
  }

  inline FontPalette ComputedStyleProperties::FontPalette() const noexcept
  {
    return GetFontDescription().GetFontPalette();
  }

  inline FontSizeAdjust ComputedStyleProperties::FontSizeAdjust() const noexcept
  {
    return GetFontDescription().GetFontSizeAdjust();
  }

  inline FontStyle ComputedStyleProperties::FontStyle() const noexcept
  {
    return {GetFontDescription().FontStyleSlope(), GetFontDescription().GetFontStyleAxis()};
  }

  inline FontOpticalSizing ComputedStyleProperties::FontOpticalSizing() const noexcept
  {
    return GetFontDescription().OpticalSizing();
  }

  inline FontFeatureSettings ComputedStyleProperties::FontFeatureSettings() const noexcept
  {
    return GetFontDescription().FeatureSettings();
  }

  inline FontVariationSettings ComputedStyleProperties::FontVariationSettings() const noexcept
  {
    return GetFontDescription().VariationSettings();
  }

  inline FontWeight ComputedStyleProperties::FontWeight() const noexcept
  {
    return GetFontDescription().Weight();
  }

  inline FontWidth ComputedStyleProperties::FontWidth() const noexcept
  {
    return GetFontDescription().Width();
  }

  inline Kerning ComputedStyleProperties::FontKerning() const noexcept
  {
    return GetFontDescription().GetKerning();
  }

  inline FontSynthesisLonghandValue ComputedStyleProperties::FontSynthesisSmallCaps() const noexcept
  {
    return GetFontDescription().FontSynthesisSmallCaps();
  }

  inline FontSynthesisLonghandValue ComputedStyleProperties::FontSynthesisStyle() const noexcept
  {
    return GetFontDescription().FontSynthesisStyle();
  }

  inline FontSynthesisLonghandValue ComputedStyleProperties::FontSynthesisWeight() const noexcept
  {
    return GetFontDescription().FontSynthesisWeight();
  }

  inline FontVariantAlternates ComputedStyleProperties::FontVariantAlternates() const noexcept
  {
    return GetFontDescription().VariantAlternates();
  }

  inline FontVariantCaps ComputedStyleProperties::FontVariantCaps() const noexcept
  {
    return GetFontDescription().VariantCaps();
  }

  inline FontVariantEastAsian ComputedStyleProperties::FontVariantEastAsian() const noexcept
  {
    return GetFontDescription().VariantEastAsian();
  }

  inline FontVariantEmoji ComputedStyleProperties::FontVariantEmoji() const noexcept
  {
    return GetFontDescription().VariantEmoji();
  }

  inline FontVariantLigatures ComputedStyleProperties::FontVariantLigatures() const noexcept
  {
    return GetFontDescription().VariantLigatures();
  }

  inline FontVariantNumeric ComputedStyleProperties::FontVariantNumeric() const noexcept
  {
    return GetFontDescription().VariantNumeric();
  }

  inline FontVariantPosition ComputedStyleProperties::FontVariantPosition() const noexcept
  {
    return GetFontDescription().VariantPosition();
  }

  inline TextRenderingMode ComputedStyleProperties::TextRendering() const noexcept
  {
    return GetFontDescription().GetTextRenderingMode();
  }

  inline TextAutospace ComputedStyleProperties::TextAutospace() const noexcept
  {
    return GetFontDescription().GetTextAutospace();
  }

  inline TextSpacingTrim ComputedStyleProperties::TextSpacingTrim() const noexcept
  {
    return GetFontDescription().GetTextSpacingTrim();
  }

  inline WebkitLocale ComputedStyleProperties::Locale() const noexcept
  {
    return GetFontDescription().SpecifiedLocale();
  }
}
