/*
 * Copyright (C) 2007 Nicholas Shanks <contact@nickshanks.com>
 * Copyright (C) 2008-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Booey/Platform/Locale/Language.hpp"
#include "Krystal.Booey/Text/LocaleToScriptMapping.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD StringAtom ComputeSpecializedChineseLocale() noexcept
    {
      for (const auto &language : UserPreferredLanguages())
      {
        if (language.size() >= 3
            && text::IsASCIICaselessEqual(utf8_stringview(language).substr(0, 3), utf8_stringview(u8"zh-")))
        {
          return StringAtom {language};
        }
      }
      return StringAtom {u8"zh-hans"}; // We have no signal. Pick one option arbitrarily.
    }

    // The first `zh-` language the user prefers, recomputed when the host sets the languages.
    const StringAtom &SpecializedChineseLocale() noexcept
    {
      static StringAtom locale = StringAtom::Null();
      static Maybe<uint32> generation;
      if (generation != UserPreferredLanguagesGeneration())
      {
        locale = ComputeSpecializedChineseLocale();
        generation = UserPreferredLanguagesGeneration();
      }
      return locale;
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
