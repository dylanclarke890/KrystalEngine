#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Core/LocaleToScriptMapping.hpp"

namespace krys::boo
{
  namespace
  {
    const StringAtom &SpecializedChineseLocale() noexcept
    {
      // FIXME: Pick the first `zh-` language of the user's preferred languages (and recompute it when they
      // change), once there are user language preferences.
      static const StringAtom specializedChineseLocale {u8"zh-hans"}; // We have no signal. Pick one option
                                                                      // arbitrarily.
      return specializedChineseLocale;
    }
  }

  void FontDescription::SetSpecifiedLocale(const StringAtom &locale) noexcept
  {
    _specifiedLocale = locale;
    _script = static_cast<uint32>(LocaleToScriptCode(_specifiedLocale.View()));
    _locale = _script == USCRIPT_HAN ? SpecializedChineseLocale() : _specifiedLocale;
  }

  StringAtom FontDescription::PlatformResolveGenericFamily(UScriptCode, const StringAtom &,
                                                           const StringAtom &) noexcept
  {
    return StringAtom::Null();
  }

  float FontDescription::AdjustedSizeForFontFace(float fontFaceSizeAdjust) const noexcept
  {
    // It is not worth modifying the used size with @font-face size-adjust if we are to re-adjust it later
    // with font-size-adjust. This is because font-size-adjust will overrule this change, since size-adjust
    // also modifies the font's metric values and thus, keeps the aspect-value unchanged.
    return GetFontSizeAdjust().Value ? ComputedSize() : fontFaceSizeAdjust * ComputedSize();
  }

  FontVariantEastAsianValues FontDescription::VariantEastAsian() const noexcept
  {
    return {
      VariantEastAsianVariant(),
      VariantEastAsianWidth(),
      VariantEastAsianRuby(),
    };
  }

  FontVariantNumericValues FontDescription::VariantNumeric() const noexcept
  {
    return {
      VariantNumericFigure(),  VariantNumericSpacing(),     VariantNumericFraction(),
      VariantNumericOrdinal(), VariantNumericSlashedZero(),
    };
  }

  FontVariantLigaturesValues FontDescription::VariantLigatures() const noexcept
  {
    return {
      VariantCommonLigatures(),
      VariantDiscretionaryLigatures(),
      VariantHistoricalLigatures(),
      VariantContextualAlternates(),
    };
  }

  FontVariantSettings FontDescription::VariantSettings() const noexcept
  {
    return {VariantCommonLigatures(),
            VariantDiscretionaryLigatures(),
            VariantHistoricalLigatures(),
            VariantContextualAlternates(),
            VariantPosition(),
            VariantCaps(),
            VariantNumericFigure(),
            VariantNumericSpacing(),
            VariantNumericFraction(),
            VariantNumericOrdinal(),
            VariantNumericSlashedZero(),
            VariantAlternates(),
            VariantEastAsianVariant(),
            VariantEastAsianWidth(),
            VariantEastAsianRuby(),
            VariantEmoji()};
  }

  void FontDescription::SetVariantEastAsian(FontVariantEastAsianValues values) noexcept
  {
    SetVariantEastAsianVariant(values.Variant);
    SetVariantEastAsianWidth(values.Width);
    SetVariantEastAsianRuby(values.Ruby);
  }

  void FontDescription::SetVariantNumeric(FontVariantNumericValues values) noexcept
  {
    SetVariantNumericFigure(values.Figure);
    SetVariantNumericSpacing(values.Spacing);
    SetVariantNumericFraction(values.Fraction);
    SetVariantNumericOrdinal(values.Ordinal);
    SetVariantNumericSlashedZero(values.SlashedZero);
  }

  void FontDescription::SetVariantLigatures(FontVariantLigaturesValues values) noexcept
  {
    SetVariantCommonLigatures(values.Common);
    SetVariantDiscretionaryLigatures(values.Discretionary);
    SetVariantHistoricalLigatures(values.Historical);
    SetVariantContextualAlternates(values.Contextual);
  }
}
