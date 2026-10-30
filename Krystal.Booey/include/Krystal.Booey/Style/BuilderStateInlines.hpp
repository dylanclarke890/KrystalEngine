#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
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
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAutospace.hpp"
#include "Krystal.Booey/Style/Values/Text/TextSpacingTrim.hpp"
#include "Krystal.Booey/Style/Values/Viewport/Zoom.hpp"

namespace krys::boo::style
{
  inline void BuilderState::SetTextOrientation(TextOrientation orientation) noexcept
  {
    _fontDirty |= _style.SetTextOrientation(orientation);
  }
  inline void BuilderState::SetWritingMode(StyleWritingMode mode) noexcept
  {
    _fontDirty |= _style.SetWritingMode(mode);
  }

  inline void BuilderState::SetZoom(Zoom zoom) noexcept
  {
    _fontDirty |= _style.SetZoom(zoom);
  }
  inline void BuilderState::SetUsedZoom(float zoom) noexcept
  {
    _fontDirty |= _style.SetUsedZoom(zoom);
  }

  inline const FontCascadeDescription &BuilderState::ParentFontDescription() const noexcept
  {
    return ParentStyle().GetFontDescription();
  }
  inline const FontCascadeDescription &BuilderState::GetFontDescription() const noexcept
  {
    return _style.GetFontDescription();
  }
  inline void BuilderState::SetFontDescription(FontCascadeDescription &&description) noexcept
  {
    _fontDirty |= _style.SetFontDescriptionWithoutUpdate(krys::move(description));
  }

  inline void BuilderState::SetFontDescriptionKeywordSizeFromIdentifier(css::ValueId identifier) noexcept
  {
    if (_style.GetFontDescription().KeywordSizeAsIdentifier() == identifier)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetKeywordSizeFromIdentifier(identifier);
  }

  inline void BuilderState::SetFontDescriptionIsAbsoluteSize(bool isAbsoluteSize) noexcept
  {
    if (_style.GetFontDescription().IsAbsoluteSize() == isAbsoluteSize)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetIsAbsoluteSize(isAbsoluteSize);
  }

  inline void BuilderState::SetFontDescriptionFontSize(float fontSize) noexcept
  {
    if (_style.GetFontDescription().SpecifiedSize() != fontSize)
    {
      _fontDirty = true;
      _style.MutableFontDescriptionWithoutUpdate().SetSpecifiedSize(fontSize);
    }

    auto computedSize = style::ComputedFontSizeFromSpecifiedSize(
      fontSize, _style.GetFontDescription().IsAbsoluteSize(), UseSVGZoomRules(), &Style(), GetDocument());
    if (_style.GetFontDescription().ComputedSize() != computedSize.Size
        || _style.GetFontDescription().UsedZoomFactor() != computedSize.UsedZoomFactor)
    {
      _fontDirty = true;
      _style.MutableFontDescriptionWithoutUpdate().SetComputedSize(computedSize.Size,
                                                                   computedSize.UsedZoomFactor);
    }
  }

  inline void BuilderState::SetFontDescriptionFamilies(FontFamilies &&families) noexcept
  {
    if (_style.GetFontDescription().Families() == families.ToPlatform()
        && _style.GetFontDescription().IsSpecifiedFont() == families.IsSpecifiedFont())
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetFamilies(families.TakePlatform());
    fontCascade.MutableFontDescription().SetIsSpecifiedFont(families.IsSpecifiedFont());
    // FIXME: `FontCascade::updateUseBackslashAsYenSymbol()` comes with the font cache.
  }

  inline void BuilderState::SetFontDescriptionFeatureSettings(FontFeatureSettings &&featureSettings) noexcept
  {
    if (_style.GetFontDescription().FeatureSettings() == featureSettings.Platform())
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetFeatureSettings(featureSettings.TakePlatform());
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionFontPalette(FontPalette &&fontPalette) noexcept
  {
    if (_style.GetFontDescription().GetFontPalette() == fontPalette.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontPalette(fontPalette.Platform());
  }

  inline void BuilderState::SetFontDescriptionFontSizeAdjust(FontSizeAdjust fontSizeAdjust) noexcept
  {
    if (_style.GetFontDescription().GetFontSizeAdjust() == fontSizeAdjust.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontSizeAdjust(fontSizeAdjust.Platform());
  }

  inline void BuilderState::SetFontDescriptionFontSmoothing(FontSmoothingMode fontSmoothing) noexcept
  {
    if (_style.GetFontDescription().FontSmoothing() == fontSmoothing)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontSmoothing(krys::move(fontSmoothing));
  }

  inline void BuilderState::SetFontDescriptionFontStyle(FontStyle fontStyle) noexcept
  {
    auto &description = _style.GetFontDescription();
    if (description.FontStyleSlope() == fontStyle.PlatformSlope()
        && description.GetFontStyleAxis() == fontStyle.PlatformAxis())
    {
      return;
    }

    _fontDirty = true;
    auto &mutableDescription = _style.MutableFontDescriptionWithoutUpdate();
    mutableDescription.SetFontStyleSlope(fontStyle.PlatformSlope());
    mutableDescription.SetFontStyleAxis(fontStyle.PlatformAxis());
  }

  inline void BuilderState::SetFontDescriptionFontSynthesisSmallCaps(
    FontSynthesisLonghandValue fontSynthesisSmallCaps) noexcept
  {
    if (_style.GetFontDescription().FontSynthesisSmallCaps() == fontSynthesisSmallCaps)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontSynthesisSmallCaps(
      krys::move(fontSynthesisSmallCaps));
  }

  inline void
    BuilderState::SetFontDescriptionFontSynthesisStyle(FontSynthesisLonghandValue fontSynthesisStyle) noexcept
  {
    if (_style.GetFontDescription().FontSynthesisStyle() == fontSynthesisStyle)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontSynthesisStyle(fontSynthesisStyle);
  }

  inline void BuilderState::SetFontDescriptionFontSynthesisWeight(
    FontSynthesisLonghandValue fontSynthesisWeight) noexcept
  {
    if (_style.GetFontDescription().FontSynthesisWeight() == fontSynthesisWeight)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetFontSynthesisWeight(fontSynthesisWeight);
  }

  inline void BuilderState::SetFontDescriptionKerning(Kerning kerning) noexcept
  {
    if (_style.GetFontDescription().GetKerning() == kerning)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetKerning(kerning);
    fontCascade.UpdateEnableKerning();
  }

  inline void BuilderState::SetFontDescriptionOpticalSizing(FontOpticalSizing opticalSizing) noexcept
  {
    if (_style.GetFontDescription().OpticalSizing() == opticalSizing)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetOpticalSizing(opticalSizing);
  }

  inline void BuilderState::SetFontDescriptionSpecifiedLocale(WebkitLocale &&specifiedLocale) noexcept
  {
    if (_style.GetFontDescription().SpecifiedLocale() == specifiedLocale.GetPlatform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetSpecifiedLocale(specifiedLocale.TakePlatform());
  }

  inline void BuilderState::SetFontDescriptionTextAutospace(TextAutospace textAutospace) noexcept
  {
    if (_style.GetFontDescription().GetTextAutospace() == ToPlatform<TextAutospace> {}(textAutospace))
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetTextAutospace(
      ToPlatform<TextAutospace> {}(textAutospace));
  }

  inline void BuilderState::SetFontDescriptionTextRenderingMode(TextRenderingMode textRenderingMode) noexcept
  {
    if (_style.GetFontDescription().GetTextRenderingMode() == textRenderingMode)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetTextRenderingMode(textRenderingMode);
    fontCascade.UpdateEnableKerning();
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionTextSpacingTrim(TextSpacingTrim textSpacingTrim) noexcept
  {
    if (_style.GetFontDescription().GetTextSpacingTrim() == textSpacingTrim.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetTextSpacingTrim(textSpacingTrim.Platform());
  }

  inline void BuilderState::SetFontDescriptionVariantCaps(FontVariantCaps variantCaps) noexcept
  {
    if (_style.GetFontDescription().VariantCaps() == variantCaps)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantCaps(variantCaps);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantEmoji(FontVariantEmoji variantEmoji) noexcept
  {
    if (_style.GetFontDescription().VariantEmoji() == variantEmoji)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantEmoji(variantEmoji);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantPosition(FontVariantPosition variantPosition) noexcept
  {
    if (_style.GetFontDescription().VariantPosition() == variantPosition)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantPosition(variantPosition);
    fontCascade.UpdateRequiresShaping();
  }

  inline void
    BuilderState::SetFontDescriptionVariationSettings(FontVariationSettings &&variationSettings) noexcept
  {
    if (_style.GetFontDescription().VariationSettings() == variationSettings.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetVariationSettings(variationSettings.TakePlatform());
  }

  inline void BuilderState::SetFontDescriptionWeight(FontWeight weight) noexcept
  {
    if (_style.GetFontDescription().Weight() == weight.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetWeight(weight.Platform());
  }

  inline void BuilderState::SetFontDescriptionWidth(FontWidth width) noexcept
  {
    if (_style.GetFontDescription().Width() == width.Platform())
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetWidth(width.Platform());
  }

  inline void
    BuilderState::SetFontDescriptionVariantAlternates(FontVariantAlternates &&variantAlternates) noexcept
  {
    if (_style.GetFontDescription().VariantAlternates() == variantAlternates.GetPlatform())
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantAlternates(variantAlternates.TakePlatform());
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantEastAsian(FontVariantEastAsian variantEastAsian) noexcept
  {
    SetFontDescriptionVariantEastAsianVariant(variantEastAsian.GetPlatform().Variant);
    SetFontDescriptionVariantEastAsianWidth(variantEastAsian.GetPlatform().Width);
    SetFontDescriptionVariantEastAsianRuby(variantEastAsian.GetPlatform().Ruby);
  }

  inline void BuilderState::SetFontDescriptionVariantEastAsianVariant(
    FontVariantEastAsianVariant variantEastAsianVariant) noexcept
  {
    if (_style.GetFontDescription().VariantEastAsianVariant() == variantEastAsianVariant)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantEastAsianVariant(variantEastAsianVariant);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantEastAsianWidth(
    FontVariantEastAsianWidth variantEastAsianWidth) noexcept
  {
    if (_style.GetFontDescription().VariantEastAsianWidth() == variantEastAsianWidth)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantEastAsianWidth(variantEastAsianWidth);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantEastAsianRuby(
    FontVariantEastAsianRuby variantEastAsianRuby) noexcept
  {
    if (_style.GetFontDescription().VariantEastAsianRuby() == variantEastAsianRuby)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantEastAsianRuby(variantEastAsianRuby);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionKeywordSize(uint32 keywordSize) noexcept
  {
    if (_style.GetFontDescription().KeywordSize() == keywordSize)
    {
      return;
    }

    _fontDirty = true;
    _style.MutableFontDescriptionWithoutUpdate().SetKeywordSize(keywordSize);
  }

  inline void BuilderState::SetFontDescriptionVariantLigatures(FontVariantLigatures variantLigatures) noexcept
  {
    SetFontDescriptionVariantCommonLigatures(variantLigatures.GetPlatform().Common);
    SetFontDescriptionVariantDiscretionaryLigatures(variantLigatures.GetPlatform().Discretionary);
    SetFontDescriptionVariantHistoricalLigatures(variantLigatures.GetPlatform().Historical);
    SetFontDescriptionVariantContextualAlternates(variantLigatures.GetPlatform().Contextual);
  }

  inline void BuilderState::SetFontDescriptionVariantCommonLigatures(
    boo::FontVariantLigatures variantCommonLigatures) noexcept
  {
    if (_style.GetFontDescription().VariantCommonLigatures() == variantCommonLigatures)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantCommonLigatures(variantCommonLigatures);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantDiscretionaryLigatures(
    boo::FontVariantLigatures variantDiscretionaryLigatures) noexcept
  {
    if (_style.GetFontDescription().VariantDiscretionaryLigatures() == variantDiscretionaryLigatures)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantDiscretionaryLigatures(variantDiscretionaryLigatures);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantHistoricalLigatures(
    boo::FontVariantLigatures variantHistoricalLigatures) noexcept
  {
    if (_style.GetFontDescription().VariantHistoricalLigatures() == variantHistoricalLigatures)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantHistoricalLigatures(variantHistoricalLigatures);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantContextualAlternates(
    boo::FontVariantLigatures variantContextualAlternates) noexcept
  {
    if (_style.GetFontDescription().VariantContextualAlternates() == variantContextualAlternates)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantContextualAlternates(variantContextualAlternates);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantNumeric(FontVariantNumeric variantNumeric) noexcept
  {
    SetFontDescriptionVariantNumericFigure(variantNumeric.GetPlatform().Figure);
    SetFontDescriptionVariantNumericSpacing(variantNumeric.GetPlatform().Spacing);
    SetFontDescriptionVariantNumericFraction(variantNumeric.GetPlatform().Fraction);
    SetFontDescriptionVariantNumericOrdinal(variantNumeric.GetPlatform().Ordinal);
    SetFontDescriptionVariantNumericSlashedZero(variantNumeric.GetPlatform().SlashedZero);
  }

  inline void BuilderState::SetFontDescriptionVariantNumericFigure(
    FontVariantNumericFigure variantNumericFigure) noexcept
  {
    if (_style.GetFontDescription().VariantNumericFigure() == variantNumericFigure)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantNumericFigure(variantNumericFigure);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantNumericSpacing(
    FontVariantNumericSpacing variantNumericSpacing) noexcept
  {
    if (_style.GetFontDescription().VariantNumericSpacing() == variantNumericSpacing)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantNumericSpacing(variantNumericSpacing);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantNumericFraction(
    FontVariantNumericFraction variantNumericFraction) noexcept
  {
    if (_style.GetFontDescription().VariantNumericFraction() == variantNumericFraction)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantNumericFraction(variantNumericFraction);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantNumericOrdinal(
    FontVariantNumericOrdinal variantNumericOrdinal) noexcept
  {
    if (_style.GetFontDescription().VariantNumericOrdinal() == variantNumericOrdinal)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantNumericOrdinal(variantNumericOrdinal);
    fontCascade.UpdateRequiresShaping();
  }

  inline void BuilderState::SetFontDescriptionVariantNumericSlashedZero(
    FontVariantNumericSlashedZero variantNumericSlashedZero) noexcept
  {
    if (_style.GetFontDescription().VariantNumericSlashedZero() == variantNumericSlashedZero)
    {
      return;
    }

    _fontDirty = true;
    auto &fontCascade = _style.MutableFontCascadeWithoutUpdate();
    fontCascade.MutableFontDescription().SetVariantNumericSlashedZero(variantNumericSlashedZero);
    fontCascade.UpdateRequiresShaping();
  }
}
