#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Bit.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Text::Unicode
{
  template <typename TChar = char32>
  constexpr inline const TChar Replacement = static_cast<TChar>(0xFFFD);
  template <typename TChar = char32>
  constexpr inline Array<const TChar, 1> ReplacementAsArray = {Replacement<TChar>};

  template <typename TChar = char32>
  constexpr inline const TChar ASCIIReplacement = static_cast<TChar>(0x003F);
  template <typename TChar = char32>
  constexpr inline Array<const TChar, 1> ASCIIReplacementAsArray = {ASCIIReplacement<TChar>};

  constexpr inline const char32 LastUnicodeCodePoint = 0x10FFFF;
  constexpr inline const char32 FirstLeadSurrogate = 0xD800;
  constexpr inline const char32 LastLeadSurrogate = 0xDBFF;
  constexpr inline const char32 FirstTrailSurrogate = 0xDC00;
  constexpr inline const char32 LastTrailSurrogate = 0xDFFF;
  constexpr inline const char32 FirstSurrogate = FirstLeadSurrogate;
  constexpr inline const char32 LastSurrogate = LastTrailSurrogate;
  constexpr inline const char32 Last1ByteValue = 0x7F;
  constexpr inline const char32 Last2ByteValue = 0x7FF;
  constexpr inline const char32 Last3ByteValue = 0xFFFF;
  constexpr inline const char32 Last4ByteValue = 0x1FFFFF;
  constexpr inline const char32 Last5ByteValue = 0x3FFFFFF;
  constexpr inline const char32 Last6ByteValue = 0x7F'FF'FF'FF;
  constexpr inline const char8 Start1ByteMask = 0x80u;
  constexpr inline const char8 Start1ByteContinuation = 0x00u;
  constexpr inline const char8 Start1ByteShift = 7u;
  constexpr inline const char8 Start2ByteMask = 0xC0u;
  constexpr inline const char8 Start2ByteContinuation = Start2ByteMask;
  constexpr inline const char8 Start2ByteShift = 5u;
  constexpr inline const char8 Start3ByteMask = 0xE0u;
  constexpr inline const char8 Start3ByteContinuation = Start3ByteMask;
  constexpr inline const char8 Start3ByteShift = 4u;
  constexpr inline const char8 Start4ByteMask = 0xF0u;
  constexpr inline const char8 Start4ByteContinuation = Start4ByteMask;
  constexpr inline const char8 Start4ByteShift = 3u;
  constexpr inline const char8 Start5ByteMask = 0xF8u;
  constexpr inline const char8 Start5ByteContinuation = Start5ByteMask;
  constexpr inline const char8 Start5ByteShift = 2u;
  constexpr inline const char8 Start6ByteMask = 0xFCu;
  constexpr inline const char8 Start6ByteContinuation = Start6ByteMask;
  constexpr inline const char8 Start6ByteShift = 1u;
  constexpr inline const char8 ContinuationMask = 0xC0u;
  constexpr inline const char8 ContinuationSignature = 0x80u;
  constexpr inline const char8 ContinuationMaskValue = 0x3Fu;
  constexpr inline const char8 SingleMaskValue = 0x7Fu;
  constexpr inline const char32 LastASCIIValue = 0x7F;
  constexpr inline const char32 LastBMPValue = 0xFFFF;
  constexpr inline const char32 NormalizingValue = 0x10000;
  constexpr inline const int LeadSurrogateBitmask = 0xFFC00;
  constexpr inline const int TrailSurrogateBitmask = 0x3FF;
  constexpr inline const int LeadShiftedBits = 10;

  KRYS_NODISCARD constexpr inline bool IsLeadSurrogate(char32 value) noexcept
  {
    return value >= FirstLeadSurrogate && value <= LastLeadSurrogate;
  }

  KRYS_NODISCARD constexpr inline bool IsTrailSurrogate(char32 value) noexcept
  {
    return value >= FirstTrailSurrogate && value <= LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr inline bool IsSurrogate(char32 value) noexcept
  {
    return value >= FirstSurrogate && value <= LastSurrogate;
  }

  KRYS_NODISCARD constexpr inline bool IsSingleUTF16(char16 value) noexcept
  {
    return value < FirstLeadSurrogate || value > LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr inline bool IsSingleOrLeadUTF16(char16 value) noexcept
  {
    return value <= LastLeadSurrogate || value > LastTrailSurrogate;
  }

  KRYS_NODISCARD constexpr inline char32 UTF16CombineSurrogates(char16 lead, char16 trail) noexcept
  {
    auto highBits = lead - FirstLeadSurrogate;
    auto lowBits = trail - FirstTrailSurrogate;
    return NormalizingValue + ((highBits << LeadShiftedBits) | lowBits);
  }

  KRYS_NODISCARD constexpr inline bool IsLeadOverlongUTF8(char8 value) noexcept
  {
    return (value & ContinuationMask) != ContinuationSignature;
  }

  KRYS_NODISCARD constexpr inline bool IsLeadUTF8(char8 value) noexcept
  {
    return (value & ContinuationMask) != ContinuationSignature && value != static_cast<char8>(0xC0)
           && value != static_cast<char8>(0xC1);
  }

  KRYS_NODISCARD constexpr inline bool IsLeadMUTF8(char8 value) noexcept
  {
    return (value & ContinuationMask) != ContinuationSignature && value != (char8)(0xC1);
  }

  KRYS_NODISCARD constexpr inline bool IsTrailingUTF8(char8 value) noexcept
  {
    return (value & ContinuationMask) == ContinuationSignature;
  }

  KRYS_NODISCARD constexpr inline bool IsNonCharacter(char32 value) noexcept
  {
    return (value >= 0xFDD0 && value <= 0xFDEF)
           || ((value & 0xFFFE) == 0xFFFE && value >= 0xFFFE && value <= LastUnicodeCodePoint);
  }

  KRYS_NODISCARD constexpr inline bool IsSingleOrLeadUTF8(char8 value) noexcept
  {
    return IsLeadUTF8(value);
  }

  KRYS_NODISCARD constexpr inline bool UTF8IsInvalid(char8 value) noexcept
  {
    return value == 0xC0 || value == 0xC1 || value > 0xF4;
  }

  KRYS_NODISCARD constexpr inline bool MUTF8IsInvalid(char8 value) noexcept
  {
    return value == 0xC1 || value > 0xF4;
  }

  KRYS_NODISCARD constexpr inline bool UTF8IsOverlong(char32 value, std::size_t bytes) noexcept
  {
    return (value <= Last1ByteValue && bytes > 1) || (value <= Last2ByteValue && bytes > 2uz)
           || (value <= Last3ByteValue && bytes > 3);
  }

  KRYS_NODISCARD constexpr inline bool UTF8IsOverlongExtended(char32 value, std::size_t bytes) noexcept
  {
    return (value <= Last1ByteValue && bytes > 1uz) || (value <= Last2ByteValue && bytes > 2uz)
           || (value <= Last3ByteValue && bytes > 3uz) || (value <= Last4ByteValue && bytes > 4uz)
           || (value <= Last5ByteValue && bytes > 5uz);
  }

  KRYS_NODISCARD constexpr inline uint8 UTF8DecodeLength(char32 value) noexcept
  {
    if (value <= Last1ByteValue)
    {
      return 1;
    }
    if (value <= Last2ByteValue)
    {
      return 2;
    }
    if (value <= Last3ByteValue)
    {
      return 3;
    }
    if (value <= Last4ByteValue)
    {
      return 4;
    }
    return 0;
  }

  KRYS_NODISCARD constexpr inline uint8 UTF8DecodeLengthOverlong(char32 value) noexcept
  {
    if (value <= Last1ByteValue)
    {
      return 1;
    }
    if (value <= Last2ByteValue)
    {
      return 2;
    }
    if (value <= Last3ByteValue)
    {
      return 3;
    }
    if (value <= Last4ByteValue)
    {
      return 4;
    }
    if (value <= Last5ByteValue)
    {
      return 5;
    }
    if (value <= Last6ByteValue)
    {
      return 6;
    }
    return 0;
  }

  KRYS_NODISCARD constexpr inline uint8 UTF8SequenceLength(char8 value) noexcept
  {
    return (value & Start1ByteMask) == Start1ByteContinuation   ? 1
           : (value & Start3ByteMask) != Start3ByteContinuation ? 2
           : (value & Start4ByteMask) != Start4ByteContinuation ? 3
                                                                : 4;
  }

  KRYS_NODISCARD constexpr inline uint8 UTF8SequenceLengthOverlong(char8 value) noexcept
  {
    return (value & Start1ByteMask) == Start1ByteContinuation   ? 1
           : (value & Start3ByteMask) != Start3ByteContinuation ? 2
           : (value & Start4ByteMask) != Start4ByteContinuation ? 3
           : (value & Start5ByteMask) != Start5ByteContinuation ? 4
           : (value & Start6ByteMask) != Start6ByteContinuation ? 5
                                                                : 6;
  }

  KRYS_NODISCARD constexpr inline char32 UTF8Decode(char8 value0, char8 value1) noexcept
  {
    return ((value0 & 0x1F) << 6) | (value1 & 0x3F);
  }

  KRYS_NODISCARD constexpr inline char32 UTF8Decode(char8 value0, char8 value1, char8 value2) noexcept
  {
    return ((value0 & 0x0F) << 12) | ((value1 & 0x3F) << 6) | (value2 & 0x3F);
  }

  KRYS_NODISCARD constexpr inline char32 UTF8Decode(char8 value0, char8 value1, char8 value2,
                                                    char8 value3) noexcept
  {
    return ((value0 & 0x07) << 18) | ((value1 & 0x3F) << 12) | ((value2 & 0x3F) << 6) | (value3 & 0x3F);
  }

  KRYS_NODISCARD constexpr inline char32 UTF8Decode(char8 value0, char8 value1, char8 value2, char8 value3,
                                                    char8 value4) noexcept
  {
    return ((value0 & 0x07) << 24) | ((value1 & 0x3F) << 18) | ((value2 & 0x3F) << 12)
           | ((value3 & 0x3F) << 6) | (value4 & 0x3F);
  }

  KRYS_NODISCARD constexpr inline char32 UTF8Decode(char8 value0, char8 value1, char8 value2, char8 value3,
                                                    char8 value4, char8 value5) noexcept
  {
    return ((value0 & 0x07) << 30) | ((value1 & 0x3F) << 24) | ((value2 & 0x3F) << 18)
           | ((value3 & 0x3F) << 12) | ((value4 & 0x3F) << 6) | (value5 & 0x3F);
  }

  template <bool IsOverlong>
  KRYS_NODISCARD constexpr inline int UTF8DecodeLength(char32 value) noexcept
  {
    if constexpr (IsOverlong)
    {
      return UTF8DecodeLengthOverlong(value);
    }
    else
    {
      return UTF8DecodeLength(value);
    }
  }
}