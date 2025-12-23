#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-8 character encoding.
  class UTF8Encoding : public Encoding
  {
  public:
    UTF8Encoding() noexcept : Encoding {"UTF-8"}
    {
    }

    virtual ~UTF8Encoding() noexcept = default;

    NO_DISCARD List<byte> GetPreamble() const noexcept override
    {
      return {byte {0xEF}, byte {0xBB}, byte {0xBF}};
    }

    NO_DISCARD bool IsSingleByte() const noexcept override
    {
      return true;
    }

    NO_DISCARD List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      bytes.reserve(characters.size());
      for (char8_t ch : characters)
      {
        byte b = static_cast<byte>(ch);
        bytes.push_back(b);
      }
      return bytes;
    }

    NO_DISCARD utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      characters.reserve(bytes.size());
      for (byte b : bytes)
      {
        char8_t ch = static_cast<char8_t>(b);
        characters.push_back(ch);
      }
      return characters;
    }
  };
}