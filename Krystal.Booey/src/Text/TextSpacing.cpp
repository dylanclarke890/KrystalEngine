/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Text/TextSpacing.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Text/CharacterProperties.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo
{
  using namespace text_spacing;

  bool TextSpacingTrim::ShouldTrimSpacing(const CharactersData &charactersData) const noexcept
  {
    switch (_trim)
    {
      case TrimType::SpaceAll: return false;
      case TrimType::Auto:     return false;
      case TrimType::TrimAll:
        return charactersData.CurrentCharacterClass == CharacterClass::FullWidthOpeningPunctuation
               || charactersData.CurrentCharacterClass == CharacterClass::FullWidthClosingPunctuation
               || charactersData.CurrentCharacterClass == CharacterClass::FullWidthMiddleDotPunctuation;
      default: return false;
    }
  }

  bool TextAutospace::ShouldApplySpacing(CharacterClass firstCharacterClass,
                                         CharacterClass secondCharacterClass) const noexcept
  {
    constexpr uint8 ideographAlphaMask =
      static_cast<uint8>(CharacterClass::Ideograph) | static_cast<uint8>(CharacterClass::NonIdeographLetter);
    constexpr uint8 ideographNumericMask =
      static_cast<uint8>(CharacterClass::Ideograph) | static_cast<uint8>(CharacterClass::NonIdeographNumeral);

    uint8 characterClassesMask =
      static_cast<uint8>(static_cast<uint8>(firstCharacterClass) | static_cast<uint8>(secondCharacterClass));
    if (HasIdeographAlpha() && characterClassesMask == ideographAlphaMask)
    {
      return true;
    }
    if (HasIdeographNumeric() && characterClassesMask == ideographNumericMask)
    {
      return true;
    }
    return false;
  }

  bool TextAutospace::ShouldApplySpacing(char32_t firstCharacter, char32_t secondCharacter) const noexcept
  {
    return ShouldApplySpacing(GetCharacterClass(firstCharacter), GetCharacterClass(secondCharacter));
  }

  float TextAutospace::TextAutospaceSize(const Font &font) noexcept
  {
    // https://www.w3.org/TR/css-text-4/#text-autospace-property
    // The amount of space introduced by these keywords is 1/8 of the CJK advance measure, i.e 0.125ic.
    return 0.125f * font.GetFontMetrics().IdeogramWidth().value_or(0);
  }

  namespace text_spacing
  {
    bool IsIdeograph(char32_t character) noexcept
    {
      // Lowest possible ideographic codepoint
      if (character < 0x2E80)
      {
        return false;
      }

      // All characters in the range of U+3041 to U+30FF, except those that belong to Unicode Punctuation [P*]
      if ((character >= 0x3041 && character <= 0x30FF) && !IsPunctuation(character))
      {
        return true;
      }
      // CJK Strokes (U+31C0 to U+31EF).
      if (character >= 0x31C0 && character <= 0x31EF)
      {
        return true;
      }
      // Katakana Phonetic Extensions (U+31F0 to U+31FF)
      if (character >= 0x31F0 && character <= 0x31FF)
      {
        return true;
      }
      if (IsOfScriptType(character, UScriptCode::USCRIPT_HAN))
      {
        return true;
      }

      return false;
    }

    KRYS_NODISCARD static bool IsNonIdeographicNumeral(char32_t character,
                                                       uint32 generalCategoryMask) noexcept
    {
      // FIXME: Should also check that it is not: upright in vertical text flow using the text-orientation
      // property or the text-combine-upright property.
      return (generalCategoryMask & U_GC_ND_MASK) && !IsEastAsianFullWidth(character);
    }

    CharacterClass GetCharacterClass(char32_t character) noexcept
    {
      // fast path
      if (text::IsASCIIAlpha(character))
      {
        return CharacterClass::NonIdeographLetter;
      }
      if (text::IsASCIIDigit(character))
      {
        return CharacterClass::NonIdeographNumeral;
      }

      auto generalCategoryMask = static_cast<uint32>(U_GET_GC_MASK(static_cast<UChar32>(character)));
      if (IsIdeograph(character))
      {
        return CharacterClass::Ideograph;
      }

      // We already know it is not an Ideograph from here
      // FIXME: Should also check that it is not: upright in vertical text flow using the text-orientation
      // property or the text-combine-upright property.
      if (generalCategoryMask & (U_GC_M_MASK | U_GC_L_MASK))
      {
        if (!IsEastAsianFullWidth(character))
        {
          return CharacterClass::NonIdeographLetter;
        }
        // General Category M/L won't apply to anything else
        return CharacterClass::Undefined;
      }

      if (IsNonIdeographicNumeral(character, generalCategoryMask))
      {
        return CharacterClass::NonIdeographNumeral;
      }

      if (generalCategoryMask & U_GC_P_MASK)
      {
        if (IsCJKSymbolOrPunctuation(character) || IsEastAsianFullWidth(character))
        {
          if (IsOpeningPunctuation(generalCategoryMask))
          {
            return CharacterClass::FullWidthOpeningPunctuation;
          }
          if (IsClosingPunctuation(generalCategoryMask))
          {
            return CharacterClass::FullWidthClosingPunctuation;
          }
        }
        if (character == text::LeftSingleQuotationMark || character == text::LeftDoubleQuotationMark)
        {
          return CharacterClass::FullWidthOpeningPunctuation;
        }
        if (character == text::RightSingleQuotationMark || character == text::RightDoubleQuotationMark)
        {
          return CharacterClass::FullWidthClosingPunctuation;
        }
      }

      if (IsFullwidthMiddleDotPunctuation(character))
      {
        return CharacterClass::FullWidthMiddleDotPunctuation;
      }
      // FIXME: implement remaining classes for text-autospace: punctuation
      return CharacterClass::Undefined;
    }

    RefPtr<Font> GetHalfWidthFontIfNeeded(const Font &font, const TextSpacingTrim &textSpacingTrim,
                                          CharactersData &charactersData) noexcept
    {
      return textSpacingTrim.ShouldTrimSpacing(charactersData) ? font.HalfWidthFont() : nullptr;
    }
  }
}
