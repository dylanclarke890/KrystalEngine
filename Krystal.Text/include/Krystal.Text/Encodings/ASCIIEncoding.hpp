#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents an ASCII character encoding.
  class ASCIIEncoding : public Encoding
  {
  public:
    constexpr ASCIIEncoding() : Encoding {"US-ASCII"}
    {
    }

    virtual ~ASCIIEncoding() = default;

    NO_DISCARD constexpr List<byte> GetPreamble() const noexcept override
    {
      return {};
    }

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return true;
    }

    NO_DISCARD constexpr size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
      return charCount;
    }

    NO_DISCARD constexpr List<byte> GetBytes(const string &characters) const noexcept override
    {

    }

    NO_DISCARD constexpr size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
      return byteCount;
    }

    NO_DISCARD constexpr string GetString(const List<byte> &bytes) const noexcept override
    {
    }
  };
}