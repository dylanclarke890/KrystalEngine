#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys
{
  class UTF8Encoding : public Encoding
  {
  public:
    static constexpr utf8_stringview Name = u8"UTF-8";
    static constexpr uint32 MIBenum = 106u;
    static constexpr uint32 WindowsCodePage = 65'001u;

    static constexpr Array<byte, 3u> BOM = {byte {0xEF}, byte {0xBB}, byte {0xBF}};

  public:
    constexpr UTF8Encoding() noexcept : Encoding({Name, MIBenum, WindowsCodePage})
    {
    }

    constexpr virtual ~UTF8Encoding() noexcept = default;

    KRYS_NODISCARD constexpr Span<const byte> GetBOM() const noexcept override
    {
      return Span<const byte> {BOM.data(), BOM.size()};
    }

    KRYS_NODISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    KRYS_NODISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size());

      const auto EncodeCodepoint = [&](UnicodeCodepoint ch, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else
        {
          Unicode::CodepointToUTF8(ch, out);
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void constexpr Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      const auto DecodeCodepoint = [&](UnicodeCodepoint ch, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
        else
        {
          Unicode::CodepointToUTF8(ch, out);
        }
      };

      Unicode::ForEachCodepoint(bytes, DecodeCodepoint);
    }
  };
}