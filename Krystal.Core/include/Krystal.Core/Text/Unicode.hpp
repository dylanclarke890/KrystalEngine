#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::text
{
  constexpr char32 UnicodeReplacementChar = 0xFFFD;
  constexpr char32 MaxUnicodeChar = 0x10FFFF;

  // Named code points.
  constexpr char32 ActivateArabicFormShaping = 0x206D;
  constexpr char32 ActivateSymmetricSwapping = 0x206B;
  constexpr char32 AegeanWordSeparatorDot = 0x10101;
  constexpr char32 AegeanWordSeparatorLine = 0x10100;
  constexpr char32 ArabicIndicPerMilleSign = 0x0609;
  constexpr char32 ArabicIndicPerTenThousandSign = 0x060A;
  constexpr char32 ArabicPercentSign = 0x066A;
  constexpr char32 BlackDownPointingSmallTriangle = 0x25BE;
  constexpr char32 BlackLeftPointingSmallTriangle = 0x25C2;
  constexpr char32 BlackRightPointingSmallTriangle = 0x25B8;
  constexpr char32 BlackSquare = 0x25A0;
  constexpr char32 BlackUpPointingSmallTriangle = 0x25B4;
  constexpr char32 Bullet = 0x2022;
  constexpr char32 CarriageReturn = 0x000D;
  constexpr char32 CjkWater = 0x6C34;
  constexpr char32 DeleteCharacter = 0x007F;
  constexpr char32 EthiopicWordspace = 0x1361;
  constexpr char32 FirstStrongIsolate = 0x2068;
  constexpr char32 FullwidthAmpersand = 0xFF06;
  constexpr char32 FullwidthCommercialAt = 0xFF20;
  constexpr char32 FullwidthNumberSign = 0xFF03;
  constexpr char32 FullwidthPercentSign = 0xFF05;
  constexpr char32 FunctionApplication = 0x2061;
  constexpr char32 HiraganaLetterSmallA = 0x3041;
  constexpr char32 InhibitArabicFormShaping = 0x206C;
  constexpr char32 InhibitSymmetricSwapping = 0x206A;
  constexpr char32 InvisibleSeparator = 0x2063;
  constexpr char32 InvisibleTimes = 0x2062;
  constexpr char32 LeftDoubleQuotationMark = 0x201C;
  constexpr char32 LeftSingleQuotationMark = 0x2018;
  constexpr char32 LeftToRightEmbed = 0x202A;
  constexpr char32 LeftToRightIsolate = 0x2066;
  constexpr char32 LeftToRightMark = 0x200E;
  constexpr char32 LeftToRightOverride = 0x202D;
  constexpr char32 LineSeparator = 0x2028;
  constexpr char32 MediumShade = 0x2592;
  constexpr char32 NarrowNoBreakSpace = 0x202F;
  constexpr char32 NationalDigitShapes = 0x206E;
  constexpr char32 NewlineCharacter = 0x000A;
  constexpr char32 NoBreakSpace = 0x00A0;
  constexpr char32 NominalDigitShapes = 0x206F;
  constexpr char32 NullCharacter = 0x0000;
  constexpr char32 ObjectReplacementCharacter = 0xFFFC;
  constexpr char32 ParagraphSeparator = 0x2029;
  constexpr char32 PartAlternationMark = 0x303D;
  constexpr char32 PerMilleSign = 0x2030;
  constexpr char32 PerTenThousandSign = 0x2031;
  constexpr char32 PilcrowSign = 0x00B6;
  constexpr char32 PopDirectionalFormatting = 0x202C;
  constexpr char32 PopDirectionalIsolate = 0x2069;
  constexpr char32 ReversedPilcrowSign = 0x204B;
  constexpr char32 RightDoubleQuotationMark = 0x201D;
  constexpr char32 RightSingleQuotationMark = 0x2019;
  constexpr char32 RightToLeftEmbed = 0x202B;
  constexpr char32 RightToLeftIsolate = 0x2067;
  constexpr char32 RightToLeftMark = 0x200F;
  constexpr char32 RightToLeftOverride = 0x202E;
  constexpr char32 SectionSign = 0x00A7;
  constexpr char32 SmallAmpersand = 0xFE60;
  constexpr char32 SmallCommercialAt = 0xFE6B;
  constexpr char32 SmallNumberSign = 0xFE5F;
  constexpr char32 SmallPercentSign = 0xFE6A;
  constexpr char32 SoftHyphen = 0x00AD;
  constexpr char32 Space = 0x0020;
  constexpr char32 SwungDash = 0x2053;
  constexpr char32 TabCharacter = 0x0009;
  constexpr char32 TibetanMarkDelimiterTshegBstar = 0x0F0C;
  constexpr char32 TibetanMarkIntersyllabicTsheg = 0x0F0B;
  constexpr char32 TironianSignEt = 0x204A;
  constexpr char32 UgariticWordDivider = 0x1039F;
  constexpr char32 WhiteBullet = 0x25E6;
  constexpr char32 WordJoiner = 0x2060;
  constexpr char32 YenSign = 0x00A5;
  constexpr char32 ZeroWidthJoiner = 0x200D;
  constexpr char32 ZeroWidthNoBreakSpace = 0xFEFF;
  constexpr char32 ZeroWidthNonJoiner = 0x200C;
  constexpr char32 ZeroWidthSpace = 0x200B;

  KRYS_NODISCARD constexpr bool IsNonUnicodeCharacter(char32 value) noexcept
  {
    return (value >= 0xFDD0 && value <= 0xFDEF)
           || ((value & 0xFFFE) == 0xFFFE && value >= 0xFFFE && value <= MaxUnicodeChar);
  }

#pragma region UTF8

  KRYS_NODISCARD constexpr bool IsInvalidUTF8(char8 value) noexcept
  {
    return value == 0xC0 || value == 0xC1 || value > 0xF4;
  }

  KRYS_NODISCARD constexpr bool IsLeadUTF8(char8 value) noexcept
  {
    krys_debug_assert(!IsInvalidUTF8(value));

    return (value & 0xC0u) != 0x80u && value != 0xC0 && value != 0xC1;
  }

  KRYS_NODISCARD constexpr bool IsTrailingUTF8(char8 value) noexcept
  {
    krys_debug_assert(!IsInvalidUTF8(value));

    return (value & 0xC0u) == 0x80u;
  }

  KRYS_NODISCARD constexpr char32 DecodeUTF8Sequence(char8 value0, char8 value1) noexcept
  {
    krys_debug_assert(IsLeadUTF8(value0));
    krys_debug_assert(IsTrailingUTF8(value1));

    return ((value0 & 0x1F) << 6) | (value1 & 0x3F);
  }

  KRYS_NODISCARD constexpr char32 DecodeUTF8Sequence(char8 value0, char8 value1, char8 value2) noexcept
  {
    krys_debug_assert(IsLeadUTF8(value0));
    krys_debug_assert(IsTrailingUTF8(value1));
    krys_debug_assert(IsTrailingUTF8(value2));

    return ((value0 & 0x0F) << 12) | ((value1 & 0x3F) << 6) | (value2 & 0x3F);
  }

  KRYS_NODISCARD constexpr char32 DecodeUTF8Sequence(char8 value0, char8 value1, char8 value2,
                                                     char8 value3) noexcept
  {
    krys_debug_assert(IsLeadUTF8(value0));
    krys_debug_assert(IsTrailingUTF8(value1));
    krys_debug_assert(IsTrailingUTF8(value2));
    krys_debug_assert(IsTrailingUTF8(value3));

    return ((value0 & 0x07) << 18) | ((value1 & 0x3F) << 12) | ((value2 & 0x3F) << 6) | (value3 & 0x3F);
  }

  KRYS_NODISCARD constexpr uint8 UTF8SequenceLength(char8 value) noexcept
  {
    krys_debug_assert(!IsInvalidUTF8(value));

    if ((value & 0x80u) == 0x00u)
    {
      return 1u;
    }

    if ((value & 0xE0u) != 0xE0u)
    {
      return 2u;
    }

    if ((value & 0xF0u) != 0xF0u)
    {
      return 3u;
    }

    return 4u;
  }

  KRYS_NODISCARD constexpr uint8 DecodeUTF8Length(char32 value) noexcept
  {
    if (value <= MaxASCIIChar)
    {
      return 1u;
    }

    if (value <= 0x7FF)
    {
      return 2u;
    }

    if (value <= 0xFFFF)
    {
      return 3u;
    }

    if (value <= MaxUnicodeChar)
    {
      return 4u;
    }

    return 0;
  }

#pragma endregion

#pragma region UTF16

  constexpr char32 FirstLeadSurrogate = 0xD800;
  constexpr char32 LastLeadSurrogate = 0xDBFF;

  constexpr char32 FirstTrailSurrogate = 0xDC00;
  constexpr char32 LastTrailSurrogate = 0xDFFF;

  constexpr char32 FirstSurrogate = FirstLeadSurrogate;
  constexpr char32 LastSurrogate = LastTrailSurrogate;

  struct SurrogatePair
  {
    char16 Lead;
    char16 Trail;
  };

  KRYS_NODISCARD constexpr bool IsSurrogate(char32 value) noexcept
  {
    return value >= FirstSurrogate && value <= LastSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsLeadSurrogate(char32 value) noexcept
  {
    return value >= FirstLeadSurrogate && value <= LastLeadSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsTrailSurrogate(char32 value) noexcept
  {
    return value >= FirstTrailSurrogate && value <= LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsSingleUTF16(char32 value) noexcept
  {
    return value < FirstLeadSurrogate || value > LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsSingleOrLeadUTF16(char32 value) noexcept
  {
    return value <= LastLeadSurrogate || value > LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr char32 UTF16CombineSurrogates(char16 lead, char16 trail) noexcept
  {
    constexpr char32 NormalizingValue = 0x10000;
    constexpr uint8 LeadShiftedBits = 10;

    auto highBits = static_cast<char32>(lead) - FirstLeadSurrogate;
    auto lowBits = static_cast<char32>(trail) - FirstTrailSurrogate;

    return NormalizingValue + ((highBits << LeadShiftedBits) | lowBits);
  }

  KRYS_NODISCARD constexpr SurrogatePair UTF16SplitSurrogates(char32 value) noexcept
  {
    krys_debug_assert(value >= 0x10000 && value <= MaxUnicodeChar);

    auto transformed = value - 0x10000;

    char16 highValue = static_cast<char16>(transformed >> 10);
    char16 lowValue = static_cast<char16>(transformed & 0b1'111'111'111);

    char16 highTag = 0b110110 << 10;
    char16 lowTag = 0b110111 << 10;

    return {static_cast<char16>(highTag | highValue), static_cast<char16>(lowTag | lowValue)};
  }

#pragma endregion
}