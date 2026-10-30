#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleBase+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties.hpp"

#include "Krystal.Booey/Animation/ViewTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"
#include "Krystal.Booey/Rendering/Style/StyleAppleColorFilterData.hpp"
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
#include "Krystal.Booey/Rendering/Style/StyleVisitedLinkColorData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"
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

namespace krys::boo::style
{
  // FIXME: Support generating properties that have their storage spread out

  inline Cursor ComputedStyleProperties::GetCursor() const noexcept
  {
    return {_rareInheritedData->CursorImages, static_cast<boo::CursorType>(_inheritedFlags.CursorType)};
  }

  inline ZIndex ComputedStyleProperties::SpecifiedZIndex() const noexcept
  {
    return _nonInheritedData->BoxData->SpecifiedZIndex();
  }

  // FIXME: Support writing mode properties.

  inline TextDirection ComputedStyleProperties::ComputedDirection() const noexcept
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
    return {List<css::CSSOMStringAtom> {GetFontDescription().Families()},
            GetFontDescription().IsSpecifiedFont()};
  }

  inline FontPalette ComputedStyleProperties::GetFontPalette() const noexcept
  {
    return GetFontDescription().GetFontPalette();
  }

  inline FontSizeAdjust ComputedStyleProperties::GetFontSizeAdjust() const noexcept
  {
    return GetFontDescription().GetFontSizeAdjust();
  }

  inline FontStyle ComputedStyleProperties::GetFontStyle() const noexcept
  {
    return {GetFontDescription().FontStyleSlope(), GetFontDescription().GetFontStyleAxis()};
  }

  inline FontOpticalSizing ComputedStyleProperties::GetFontOpticalSizing() const noexcept
  {
    return GetFontDescription().OpticalSizing();
  }

  inline FontFeatureSettings ComputedStyleProperties::GetFontFeatureSettings() const noexcept
  {
    return GetFontDescription().FeatureSettings();
  }

  inline FontVariationSettings ComputedStyleProperties::GetFontVariationSettings() const noexcept
  {
    return GetFontDescription().VariationSettings();
  }

  inline FontWeight ComputedStyleProperties::GetFontWeight() const noexcept
  {
    return GetFontDescription().Weight();
  }

  inline FontWidth ComputedStyleProperties::GetFontWidth() const noexcept
  {
    return GetFontDescription().Width();
  }

  inline Kerning ComputedStyleProperties::FontKerning() const noexcept
  {
    return GetFontDescription().GetKerning();
  }

  inline FontSmoothingMode ComputedStyleProperties::FontSmoothing() const noexcept
  {
    return GetFontDescription().FontSmoothing();
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

  inline FontVariantAlternates ComputedStyleProperties::GetFontVariantAlternates() const noexcept
  {
    return GetFontDescription().VariantAlternates();
  }

  inline FontVariantCaps ComputedStyleProperties::GetFontVariantCaps() const noexcept
  {
    return GetFontDescription().VariantCaps();
  }

  inline FontVariantEastAsian ComputedStyleProperties::GetFontVariantEastAsian() const noexcept
  {
    return GetFontDescription().VariantEastAsian();
  }

  inline FontVariantEmoji ComputedStyleProperties::GetFontVariantEmoji() const noexcept
  {
    return GetFontDescription().VariantEmoji();
  }

  inline FontVariantLigatures ComputedStyleProperties::GetFontVariantLigatures() const noexcept
  {
    return GetFontDescription().VariantLigatures();
  }

  inline FontVariantNumeric ComputedStyleProperties::GetFontVariantNumeric() const noexcept
  {
    return GetFontDescription().VariantNumeric();
  }

  inline FontVariantPosition ComputedStyleProperties::GetFontVariantPosition() const noexcept
  {
    return GetFontDescription().VariantPosition();
  }

  inline TextRenderingMode ComputedStyleProperties::TextRendering() const noexcept
  {
    return GetFontDescription().GetTextRenderingMode();
  }

  inline TextAutospace ComputedStyleProperties::GetTextAutospace() const noexcept
  {
    return GetFontDescription().GetTextAutospace();
  }

  inline TextSpacingTrim ComputedStyleProperties::GetTextSpacingTrim() const noexcept
  {
    return GetFontDescription().GetTextSpacingTrim();
  }

  inline WebkitLocale ComputedStyleProperties::GetLocale() const noexcept
  {
    return GetFontDescription().SpecifiedLocale();
  }
}
