#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <unicode/uchar.h>
#include <unicode/uscript.h>

namespace krys::boo
{
  KRYS_NODISCARD inline bool IsEmojiGroupCandidate(char32 character) noexcept
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

  KRYS_NODISCARD inline bool IsEmojiWithPresentationByDefault(char32 character) noexcept
  {
    // No characters in Latin-1 include "Emoji_Presentation"
    // https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt
    if (character <= 0xFF)
    {
      return false;
    }
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_EMOJI_PRESENTATION);
  }

  KRYS_NODISCARD inline bool IsEmojiModifierBase(char32 character) noexcept
  {
    // No characters in Latin-1 include "Emoji_Modifier_Base"
    // https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt
    if (character <= 0xFF)
    {
      return false;
    }
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_EMOJI_MODIFIER_BASE);
  }

  KRYS_NODISCARD inline bool IsDefaultIgnorableCodePoint(char32 character) noexcept
  {
    return u_hasBinaryProperty(static_cast<UChar32>(character), UCHAR_DEFAULT_IGNORABLE_CODE_POINT);
  }

  KRYS_NODISCARD inline bool IsControlCharacter(char32 character) noexcept
  {
    return u_charType(static_cast<UChar32>(character)) == U_CONTROL_CHAR;
  }

  KRYS_NODISCARD inline bool IsPrivateUseAreaCharacter(char32 character) noexcept
  {
    auto block = ublock_getCode(static_cast<UChar32>(character));
    return block == UBLOCK_PRIVATE_USE_AREA || block == UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A
           || block == UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B;
  }

  KRYS_NODISCARD inline bool IsPunctuation(char32 character) noexcept
  {
    return U_GET_GC_MASK(static_cast<UChar32>(character)) & U_GC_P_MASK;
  }

  KRYS_NODISCARD constexpr bool IsOpeningPunctuation(uint32 generalCategoryMask) noexcept
  {
    return generalCategoryMask & U_GC_PS_MASK;
  }

  KRYS_NODISCARD constexpr bool IsClosingPunctuation(uint32 generalCategoryMask) noexcept
  {
    return generalCategoryMask & U_GC_PE_MASK;
  }

  KRYS_NODISCARD inline bool IsOfScriptType(char32 codePoint, UScriptCode scriptType) noexcept
  {
    UErrorCode error = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(static_cast<UChar32>(codePoint), &error);
    if (error != U_ZERO_ERROR)
    {
      return false;
    }
    return script == scriptType;
  }

  KRYS_NODISCARD inline UEastAsianWidth EastAsianWidth(char32 character) noexcept
  {
    return static_cast<UEastAsianWidth>(
      u_getIntPropertyValue(static_cast<UChar32>(character), UCHAR_EAST_ASIAN_WIDTH));
  }

  KRYS_NODISCARD inline bool IsEastAsianFullWidth(char32 character) noexcept
  {
    return EastAsianWidth(character) == UEastAsianWidth::U_EA_FULLWIDTH;
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
    return u_charType(static_cast<UChar32>(character)) == U_CURRENCY_SYMBOL;
  }
}
