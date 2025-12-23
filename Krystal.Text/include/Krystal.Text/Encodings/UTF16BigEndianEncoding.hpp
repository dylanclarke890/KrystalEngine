#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-16 big-endian character encoding.
  class UTF16BigEndianEncoding : public Encoding
  {
  public:
    constexpr UTF16BigEndianEncoding() : Encoding {"UTF-16BE"}
    {
    }

    virtual ~UTF16BigEndianEncoding() = default;

    NO_DISCARD constexpr List<byte> GetPreamble() const noexcept override
    {
      return {byte {0xFE}, byte {0xFF}};
    }

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return false;
    }

    NO_DISCARD constexpr size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
    }

    NO_DISCARD constexpr List<byte> GetBytes(const string &characters) const noexcept override
    {
    }

    NO_DISCARD constexpr size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
    }

    NO_DISCARD constexpr string GetString(const List<byte> &bytes) const noexcept override
    {
    }
  };
}