#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/EncodingId.hpp"

namespace Krys::Text::BOM
{
  constexpr static inline const Array<const byte, 3u> UTF8 = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
  constexpr static inline const Array<const byte, 2u> UTF16BE = {byte {0xFE}, byte {0xFF}};
  constexpr static inline const Array<const byte, 2u> UTF16LE = {byte {0xFF}, byte {0xFE}};
  constexpr static inline const Array<const byte, 4u> UTF32BE = {byte {0x00}, byte {0x00}, byte {0xFF},
                                                                 byte {0xFE}};
  constexpr static inline const Array<const byte, 4u> UTF32LE = {byte {0xFE}, byte {0xFF}, byte {0x00},
                                                                 byte {0x00}};

  KRYS_NODISCARD constexpr inline EncodingId DetectByteOrderMark(Span<const byte> bytes) noexcept
  {
    if (ByteUtils::Compare(bytes, FixedSpan<const byte, 3u>(UTF8.data(), 3u)))
    {
      return EncodingId::utf8;
    }

    if (ByteUtils::Compare(bytes, FixedSpan<const byte, 4u>(UTF32BE.data(), 4u)))
    {
      return EncodingId::utf32be;
    }

    if (ByteUtils::Compare(bytes, FixedSpan<const byte, 4u>(UTF32LE.data(), 4u)))
    {
      return EncodingId::utf32le;
    }

    if (ByteUtils::Compare(bytes, FixedSpan<const byte, 2u>(UTF16BE.data(), 2u)))
    {
      return EncodingId::utf16be;
    }

    if (ByteUtils::Compare(bytes, FixedSpan<const byte, 2u>(UTF16LE.data(), 2u)))
    {
      return EncodingId::utf16le;
    }

    return EncodingId::unknown;
  }

  KRYS_NODISCARD constexpr inline Span<const byte> GetBytes(EncodingId encoding) noexcept
  {
    switch (encoding)
    {
      case EncodingId::utf8:    return Span<const byte> {UTF8.data(), UTF8.size()};
      case EncodingId::utf16be: return Span<const byte> {UTF16BE.data(), UTF16BE.size()};
      case EncodingId::utf16le: return Span<const byte> {UTF16LE.data(), UTF16LE.size()};
      case EncodingId::utf32be: return Span<const byte> {UTF32BE.data(), UTF32BE.size()};
      case EncodingId::utf32le: return Span<const byte> {UTF32LE.data(), UTF32LE.size()};
      default:                  return Span<const byte> {};
    }
  }
}