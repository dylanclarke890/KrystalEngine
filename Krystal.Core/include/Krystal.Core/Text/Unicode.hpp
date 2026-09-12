#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::text
{
  constexpr char32 UnicodeReplacementChar = 0xFFFD;
  constexpr char32 MaxUnicodeChar = 0x10FFFF;

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

  KRYS_NODISCARD constexpr bool IsSurrogate(char16 value) noexcept
  {
    return value >= FirstSurrogate && value <= LastSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsLeadSurrogate(char16 value) noexcept
  {
    return value >= FirstLeadSurrogate && value <= LastLeadSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsTrailSurrogate(char16 value) noexcept
  {
    return value >= FirstTrailSurrogate && value <= LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsSingleUTF16(char16 value) noexcept
  {
    return value < FirstLeadSurrogate || value > LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr bool IsSingleOrLeadUTF16(char16 value) noexcept
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

    char16 highValue = transformed >> 10;
    char16 lowValue = transformed & 0b1'111'111'111;

    char16 highTag = 0b110110 << 10;
    char16 lowTag = 0b110111 << 10;

    return {static_cast<char16>(highTag | highValue), static_cast<char16>(lowTag | lowValue)};
  }

#pragma endregion
}