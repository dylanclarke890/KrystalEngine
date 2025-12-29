#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys
{
  enum class ByteOrderMark
  {
    None,
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
  };

  struct ByteOrderMarkScanner : NonCopyMovable<ByteOrderMarkScanner>
  {
    static constexpr Array<const byte, 3u> UTF8 = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
    static constexpr Array<const byte, 2u> UTF16BE = {byte {0xFE}, byte {0xFF}};
    static constexpr Array<const byte, 2u> UTF16LE = {byte {0xFF}, byte {0xFE}};
    static constexpr Array<byte, 4u> UTF32BE = {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
    static constexpr Array<byte, 4u> UTF32LE = {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}};

    KRYS_NODISCARD constexpr static ByteOrderMark Detect(Span<const byte> bytes) noexcept
    {
      if (CompareBytes(bytes, FixedSpan<const byte, 3u>(UTF8.data(), 3u)))
      {
        return ByteOrderMark::UTF8;
      }

      if (CompareBytes(bytes, FixedSpan<const byte, 4u>(UTF32BE.data(), 4u)))
      {
        return ByteOrderMark::UTF32BE;
      }

      if (CompareBytes(bytes, FixedSpan<const byte, 4u>(UTF32LE.data(), 4u)))
      {
        return ByteOrderMark::UTF32LE;
      }

      if (CompareBytes(bytes, FixedSpan<const byte, 2u>(UTF16BE.data(), 2u)))
      {
        return ByteOrderMark::UTF16BE;
      }

      if (CompareBytes(bytes, FixedSpan<const byte, 2u>(UTF16LE.data(), 2u)))
      {
        return ByteOrderMark::UTF16LE;
      }

      return ByteOrderMark::None;
    }

    KRYS_NODISCARD constexpr static utf8_string GetEncodingName(ByteOrderMark bom) noexcept
    {
      switch (bom)
      {
        case ByteOrderMark::UTF8:    return u8"UTF-8";
        case ByteOrderMark::UTF16BE: return u8"UTF-16BE";
        case ByteOrderMark::UTF16LE: return u8"UTF-16LE";
        case ByteOrderMark::UTF32BE: return u8"UTF-32BE";
        case ByteOrderMark::UTF32LE: return u8"UTF-32LE";
        default:                     return u8"";
      }
    }

    KRYS_NODISCARD constexpr static Span<const byte> GetBytes(ByteOrderMark bom) noexcept
    {
      switch (bom)
      {
        case ByteOrderMark::UTF8:    return Span<const byte> {UTF8.data(), UTF8.size()};
        case ByteOrderMark::UTF16BE: return Span<const byte> {UTF16BE.data(), UTF16BE.size()};
        case ByteOrderMark::UTF16LE: return Span<const byte> {UTF16LE.data(), UTF16LE.size()};
        case ByteOrderMark::UTF32BE: return Span<const byte> {UTF32BE.data(), UTF32BE.size()};
        case ByteOrderMark::UTF32LE: return Span<const byte> {UTF32LE.data(), UTF32LE.size()};
        default:                     return Span<const byte> {};
      }
    }

  private:
    template <size_t N>
    KRYS_NODISCARD constexpr static bool CompareBytes(Span<const byte> bytes,
                                                      FixedSpan<const byte, N> bom) noexcept
    {
      if (bytes.size() < N)
      {
        return false;
      }

      for (size_t i = 0; i < N; ++i)
      {
        if (bytes[i] != bom[i])
        {
          return false;
        }
      }

      return true;
    }
  };
}