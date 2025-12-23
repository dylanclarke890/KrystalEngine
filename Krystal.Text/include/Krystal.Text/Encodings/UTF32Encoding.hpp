#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-32 little-endian character encoding.
  class UTF32Encoding : public Encoding
  {
  private:
    bool _isBigEndian;

  public:
    UTF32Encoding(bool isBigEndian) noexcept
        : Encoding {isBigEndian ? "UTF-32BE" : "UTF-32LE"}, _isBigEndian(isBigEndian)
    {
    }

    virtual ~UTF32Encoding() noexcept = default;

    NO_DISCARD constexpr List<byte> GetPreamble() const noexcept override
    {
      return _isBigEndian ? List<byte> {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}}
                          : List<byte> {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
    }

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return false;
    }

    NO_DISCARD constexpr size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
    }

    NO_DISCARD constexpr size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
    }

    NO_DISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
    }

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
    }
  };
}