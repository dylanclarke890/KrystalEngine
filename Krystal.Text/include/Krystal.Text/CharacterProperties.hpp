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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Text/Script.hpp"

namespace krys::text
{
  /// @brief A character's General_Category property, in ICU's `UCharCategory` order.
  enum class GeneralCategory : uint8
  {
    Unassigned,
    UppercaseLetter,
    LowercaseLetter,
    TitlecaseLetter,
    ModifierLetter,
    OtherLetter,
    NonspacingMark,
    EnclosingMark,
    SpacingMark,
    DecimalNumber,
    LetterNumber,
    OtherNumber,
    SpaceSeparator,
    LineSeparator,
    ParagraphSeparator,
    Control,
    Format,
    PrivateUse,
    Surrogate,
    DashPunctuation,
    OpenPunctuation,
    ClosePunctuation,
    ConnectorPunctuation,
    OtherPunctuation,
    MathSymbol,
    CurrencySymbol,
    ModifierSymbol,
    OtherSymbol,
    InitialPunctuation,
    FinalPunctuation,
  };

  using GeneralCategorySet = EnumSet<GeneralCategory>;

  /// @brief The categories of general category L.
  constexpr GeneralCategorySet LetterCategories {
    GeneralCategory::UppercaseLetter, GeneralCategory::LowercaseLetter, GeneralCategory::TitlecaseLetter,
    GeneralCategory::ModifierLetter, GeneralCategory::OtherLetter};

  /// @brief The categories of general category M.
  constexpr GeneralCategorySet MarkCategories {GeneralCategory::NonspacingMark,
                                               GeneralCategory::EnclosingMark, GeneralCategory::SpacingMark};

  /// @brief The categories of general category N.
  constexpr GeneralCategorySet NumberCategories {GeneralCategory::DecimalNumber,
                                                 GeneralCategory::LetterNumber, GeneralCategory::OtherNumber};

  /// @brief The categories of general category P.
  constexpr GeneralCategorySet PunctuationCategories {
    GeneralCategory::DashPunctuation,  GeneralCategory::OpenPunctuation,
    GeneralCategory::ClosePunctuation, GeneralCategory::ConnectorPunctuation,
    GeneralCategory::OtherPunctuation, GeneralCategory::InitialPunctuation,
    GeneralCategory::FinalPunctuation};

  /// @brief The categories of general category S.
  constexpr GeneralCategorySet SymbolCategories {GeneralCategory::MathSymbol, GeneralCategory::CurrencySymbol,
                                                 GeneralCategory::ModifierSymbol,
                                                 GeneralCategory::OtherSymbol};

  /// @brief The categories of general category Z.
  constexpr GeneralCategorySet SeparatorCategories {
    GeneralCategory::SpaceSeparator, GeneralCategory::LineSeparator, GeneralCategory::ParagraphSeparator};

  /// @brief The categories of general category C.
  constexpr GeneralCategorySet OtherCategories {GeneralCategory::Unassigned, GeneralCategory::Control,
                                                GeneralCategory::Format, GeneralCategory::PrivateUse,
                                                GeneralCategory::Surrogate};

  /// @brief @p character's General_Category property.
  KRYS_NODISCARD GeneralCategory GetGeneralCategory(char32 character) noexcept;

  /// @brief Whether @p character's general category is one of @p categories.
  KRYS_NODISCARD inline bool IsInGeneralCategory(char32 character, GeneralCategorySet categories) noexcept
  {
    return categories.contains(GetGeneralCategory(character));
  }

  /// @brief A character's East_Asian_Width property (UAX #11), in ICU's `UEastAsianWidth` order.
  enum class EastAsianWidth : uint8
  {
    Neutral,
    Ambiguous,
    Halfwidth,
    Fullwidth,
    Narrow,
    Wide,
  };

  /// @brief @p character's East_Asian_Width property.
  KRYS_NODISCARD EastAsianWidth GetEastAsianWidth(char32 character) noexcept;

  /// @brief Whether @p character is in one of the emoji and pictograph blocks.
  KRYS_NODISCARD bool IsEmojiGroupCandidate(char32 character) noexcept;

  KRYS_NODISCARD constexpr bool IsEmojiFitzpatrickModifier(char32 character) noexcept
  {
    // U+1F3FB - EMOJI MODIFIER FITZPATRICK TYPE-1-2
    // U+1F3FC - EMOJI MODIFIER FITZPATRICK TYPE-3
    // U+1F3FD - EMOJI MODIFIER FITZPATRICK TYPE-4
    // U+1F3FE - EMOJI MODIFIER FITZPATRICK TYPE-5
    // U+1F3FF - EMOJI MODIFIER FITZPATRICK TYPE-6

    return character >= 0x1F3FB && character <= 0x1F3FF;
  }

  KRYS_NODISCARD constexpr bool IsVariationSelector(char32 character) noexcept
  {
    return character >= 0xFE00 && character <= 0xFE0F;
  }

  KRYS_NODISCARD constexpr bool IsEmojiKeycapBase(char32 character) noexcept
  {
    return (character >= U'0' && character <= U'9') || character == U'#' || character == U'*';
  }

  KRYS_NODISCARD constexpr bool IsEmojiRegionalIndicator(char32 character) noexcept
  {
    return character >= 0x1F1E6 && character <= 0x1F1FF;
  }

  KRYS_NODISCARD bool IsEmojiWithPresentationByDefault(char32 character) noexcept;

  KRYS_NODISCARD bool IsEmojiModifierBase(char32 character) noexcept;

  KRYS_NODISCARD bool IsDefaultIgnorableCodePoint(char32 character) noexcept;

  KRYS_NODISCARD inline bool IsControlCharacter(char32 character) noexcept
  {
    return GetGeneralCategory(character) == GeneralCategory::Control;
  }

  KRYS_NODISCARD bool IsPrivateUseAreaCharacter(char32 character) noexcept;

  KRYS_NODISCARD inline bool IsPunctuation(char32 character) noexcept
  {
    return IsInGeneralCategory(character, PunctuationCategories);
  }

  KRYS_NODISCARD constexpr bool IsOpeningPunctuation(GeneralCategory category) noexcept
  {
    return category == GeneralCategory::OpenPunctuation;
  }

  KRYS_NODISCARD constexpr bool IsClosingPunctuation(GeneralCategory category) noexcept
  {
    return category == GeneralCategory::ClosePunctuation;
  }

  KRYS_NODISCARD inline bool IsOfScriptType(char32 codePoint, Script scriptType) noexcept
  {
    return GetScript(codePoint) == scriptType;
  }

  KRYS_NODISCARD inline bool IsEastAsianFullWidth(char32 character) noexcept
  {
    return GetEastAsianWidth(character) == EastAsianWidth::Fullwidth;
  }

  KRYS_NODISCARD constexpr bool IsCJKSymbolOrPunctuation(char32 character) noexcept
  {
    // CJK Symbols and Punctuation block (U+3000–U+303F)
    return character >= 0x3000 && character <= 0x303F;
  }

  KRYS_NODISCARD constexpr bool IsFullwidthMiddleDotPunctuation(char32 character) noexcept
  {
    // U+00B7 MIDDLE DOT
    // U+2027 HYPHENATION POINT
    // U+30FB KATAKANA MIDDLE DOT
    return character == 0x00B7 || character == 0x2027 || character == 0x30FB;
  }

  KRYS_NODISCARD constexpr bool IsCombiningMark(char32 character) noexcept
  {
    return 0x0300 <= character && character <= 0x036F;
  }

  KRYS_NODISCARD inline bool IsCurrencySymbol(char32 character) noexcept
  {
    return GetGeneralCategory(character) == GeneralCategory::CurrencySymbol;
  }

  /// @brief Whether @p character is printable: a graphic character or a space separator.
  KRYS_NODISCARD bool IsPrintable(char32 character) noexcept;

  /// @brief Whether @p character has the White_Space property.
  KRYS_NODISCARD bool IsWhiteSpace(char32 character) noexcept;

  /// @brief Whether @p character is in one of the CJK, Hangul, Kana, Bopomofo or ideograph blocks, which hold
  /// the Han ideographs and the Japanese and Korean characters set beside them.
  KRYS_NODISCARD bool IsInCJKBlock(char32 character) noexcept;

  /// @brief @p character's full-width form, or itself when it has none: what its `<narrow>` decomposition
  /// maps it to, or the character whose `<wide>` decomposition maps to it (as in Firefox and Safari, and the
  /// tests of CSS Text's `text-transform: full-width`).
  KRYS_NODISCARD char32 FullWidthForm(char32 character) noexcept;
}
