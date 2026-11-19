/*
 * Copyright (C) 2015-2024 Apple, Inc. All rights reserved.
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

#include "Krystal.Text/CharacterProperties.hpp"
#include <unicode/uchar.h>

namespace krys::text
{
  static_assert(static_cast<int>(GeneralCategory::Unassigned) == U_UNASSIGNED);
  static_assert(static_cast<int>(GeneralCategory::UppercaseLetter) == U_UPPERCASE_LETTER);
  static_assert(static_cast<int>(GeneralCategory::LowercaseLetter) == U_LOWERCASE_LETTER);
  static_assert(static_cast<int>(GeneralCategory::TitlecaseLetter) == U_TITLECASE_LETTER);
  static_assert(static_cast<int>(GeneralCategory::ModifierLetter) == U_MODIFIER_LETTER);
  static_assert(static_cast<int>(GeneralCategory::OtherLetter) == U_OTHER_LETTER);
  static_assert(static_cast<int>(GeneralCategory::NonspacingMark) == U_NON_SPACING_MARK);
  static_assert(static_cast<int>(GeneralCategory::EnclosingMark) == U_ENCLOSING_MARK);
  static_assert(static_cast<int>(GeneralCategory::SpacingMark) == U_COMBINING_SPACING_MARK);
  static_assert(static_cast<int>(GeneralCategory::DecimalNumber) == U_DECIMAL_DIGIT_NUMBER);
  static_assert(static_cast<int>(GeneralCategory::LetterNumber) == U_LETTER_NUMBER);
  static_assert(static_cast<int>(GeneralCategory::OtherNumber) == U_OTHER_NUMBER);
  static_assert(static_cast<int>(GeneralCategory::SpaceSeparator) == U_SPACE_SEPARATOR);
  static_assert(static_cast<int>(GeneralCategory::LineSeparator) == U_LINE_SEPARATOR);
  static_assert(static_cast<int>(GeneralCategory::ParagraphSeparator) == U_PARAGRAPH_SEPARATOR);
  static_assert(static_cast<int>(GeneralCategory::Control) == U_CONTROL_CHAR);
  static_assert(static_cast<int>(GeneralCategory::Format) == U_FORMAT_CHAR);
  static_assert(static_cast<int>(GeneralCategory::PrivateUse) == U_PRIVATE_USE_CHAR);
  static_assert(static_cast<int>(GeneralCategory::Surrogate) == U_SURROGATE);
  static_assert(static_cast<int>(GeneralCategory::DashPunctuation) == U_DASH_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::OpenPunctuation) == U_START_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::ClosePunctuation) == U_END_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::ConnectorPunctuation) == U_CONNECTOR_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::OtherPunctuation) == U_OTHER_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::MathSymbol) == U_MATH_SYMBOL);
  static_assert(static_cast<int>(GeneralCategory::CurrencySymbol) == U_CURRENCY_SYMBOL);
  static_assert(static_cast<int>(GeneralCategory::ModifierSymbol) == U_MODIFIER_SYMBOL);
  static_assert(static_cast<int>(GeneralCategory::OtherSymbol) == U_OTHER_SYMBOL);
  static_assert(static_cast<int>(GeneralCategory::InitialPunctuation) == U_INITIAL_PUNCTUATION);
  static_assert(static_cast<int>(GeneralCategory::FinalPunctuation) == U_FINAL_PUNCTUATION);

  static_assert(LetterCategories.ToRaw() == U_GC_L_MASK);
  static_assert(MarkCategories.ToRaw() == U_GC_M_MASK);
  static_assert(NumberCategories.ToRaw() == U_GC_N_MASK);
  static_assert(PunctuationCategories.ToRaw() == U_GC_P_MASK);
  static_assert(SymbolCategories.ToRaw() == U_GC_S_MASK);
  static_assert(SeparatorCategories.ToRaw() == U_GC_Z_MASK);
  static_assert(OtherCategories.ToRaw() == U_GC_C_MASK);

  static_assert(static_cast<int>(EastAsianWidth::Neutral) == U_EA_NEUTRAL);
  static_assert(static_cast<int>(EastAsianWidth::Ambiguous) == U_EA_AMBIGUOUS);
  static_assert(static_cast<int>(EastAsianWidth::Halfwidth) == U_EA_HALFWIDTH);
  static_assert(static_cast<int>(EastAsianWidth::Fullwidth) == U_EA_FULLWIDTH);
  static_assert(static_cast<int>(EastAsianWidth::Narrow) == U_EA_NARROW);
  static_assert(static_cast<int>(EastAsianWidth::Wide) == U_EA_WIDE);

  GeneralCategory GetGeneralCategory(char32 character) noexcept
  {
    return static_cast<GeneralCategory>(u_charType(static_cast<UChar32>(character)));
  }

  EastAsianWidth GetEastAsianWidth(char32 character) noexcept
  {
    return static_cast<EastAsianWidth>(
      u_getIntPropertyValue(static_cast<UChar32>(character), UCHAR_EAST_ASIAN_WIDTH));
  }

  bool IsEmojiGroupCandidate(char32 character) noexcept
  {
    switch (ublock_getCode(static_cast<UChar32>(character)))
    {
      case UBLOCK_MISCELLANEOUS_SYMBOLS:
      case UBLOCK_DINGBATS:
      case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS:
      case UBLOCK_EMOTICONS:
      case UBLOCK_TRANSPORT_AND_MAP_SYMBOLS:
      case UBLOCK_SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS:
      case UBLOCK_SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A:    return true;
      default:                                           return false;
    }
  }

  bool IsEmojiWithPresentationByDefault(char32 character) noexcept
  {
    // No characters in Latin-1 include "Emoji_Presentation"
    // https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt
    if (character <= 0xFF)
    {
      return false;
    }
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_EMOJI_PRESENTATION);
  }

  bool IsEmojiModifierBase(char32 character) noexcept
  {
    // No characters in Latin-1 include "Emoji_Modifier_Base"
    // https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt
    if (character <= 0xFF)
    {
      return false;
    }
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_EMOJI_MODIFIER_BASE);
  }

  bool IsDefaultIgnorableCodePoint(char32 character) noexcept
  {
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_DEFAULT_IGNORABLE_CODE_POINT);
  }

  bool IsPrivateUseAreaCharacter(char32 character) noexcept
  {
    auto block = ublock_getCode(static_cast<UChar32>(character));
    return block == UBLOCK_PRIVATE_USE_AREA || block == UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A
           || block == UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B;
  }
}
