#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-16 big-endian character encoding.
  class UTF16Encoding : public Encoding
  {
  private:
    bool _isBigEndian;

  public:
    UTF16Encoding(bool isBigEndian) noexcept
        : Encoding {isBigEndian ? "UTF-16BE" : "UTF-16LE"}, _isBigEndian(isBigEndian)
    {
    }

    virtual ~UTF16Encoding() noexcept = default;

    NO_DISCARD List<byte> GetPreamble() const noexcept override
    {
      return _isBigEndian ? List<byte> {byte {0xFE}, byte {0xFF}} : List<byte> {byte {0xFF}, byte {0xFE}};
    }

    NO_DISCARD bool IsSingleByte() const noexcept override
    {
      return false;
    }

    NO_DISCARD size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
    }

    NO_DISCARD size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
    }

    NO_DISCARD List<byte> Encode(utf8_stringview characters) const noexcept override
    {
    }

    NO_DISCARD utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
    }
  };
}