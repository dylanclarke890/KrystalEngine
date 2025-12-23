#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Text
{
  /// @brief Represents a character encoding.
  class Encoding
  {
  protected:
    /// @brief The IANA name of the encoding.
    string _ianaName;

    constexpr explicit Encoding(const string &name) : _ianaName(name)
    {
    }

  public:
    virtual ~Encoding() = default;

    /// @brief When overridden in a derived class, returns a sequence of bytes that specifies the encoding
    /// used.
    NO_DISCARD constexpr virtual List<byte> GetPreamble() const noexcept
    {
      return {};
    }

    /// @brief When overridden in a derived class, indicates whether the encoding uses single-byte characters.
    NO_DISCARD constexpr virtual bool IsSingleByte() const noexcept
    {
      return false;
    }

    /// @brief When overridden in a derived class, calculates the maximum number of bytes produced by encoding
    /// the specified number of characters.
    NO_DISCARD constexpr virtual size_t GetMaxByteCount(size_t charCount) const noexcept = 0;

    /// @brief When overridden in a derived class, encodes a string into a sequence of bytes.
    NO_DISCARD constexpr virtual List<byte> GetBytes(const string &characters) const noexcept = 0;

    /// @brief When overridden in a derived class, calculates the maximum number of characters produced by
    /// decoding the specified number of bytes.
    NO_DISCARD constexpr virtual size_t GetMaxCharCount(size_t byteCount) const noexcept = 0;

    /// @brief When overridden in a derived class, decodes a sequence of bytes into a string.
    NO_DISCARD constexpr virtual string GetString(const List<byte> &bytes) const noexcept = 0;
  };
}