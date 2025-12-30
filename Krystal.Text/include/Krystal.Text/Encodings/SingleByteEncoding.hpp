#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class SingleByteEncoding : public Encoding
  {
  protected:
    class LookupTable
    {
    public:
      using ASCIIToCodepointTable = Array<Rune, 128u>;
      using CodepointToASCIITable = Map<Rune, uint8, StronglyTypedNumberHasher<Rune>>;

      using MapItem = Pair<uint8, Rune>;
      using Mapping = Array<MapItem, 128u>;

    private:
      ASCIIToCodepointTable _table {};
      CodepointToASCIITable _reverseTable {};

    public:
      explicit LookupTable(const Mapping &mappings) noexcept
      {
        for (const auto [ch, codepoint] : mappings)
        {
          const auto index = ch - ASCII::ExtendedASCIIStart;
          _table[index] = codepoint;
          _reverseTable[codepoint] = ch;
        }
      }

      KRYS_NODISCARD bool TryEncode(Rune codepoint, uint8 &out) const noexcept
      {
        if (ASCII::IsASCII(codepoint))
        {
          out = static_cast<uint8>(codepoint);
          return true;
        }

        const auto it = _reverseTable.find(codepoint);
        if (it != _reverseTable.end())
        {
          out = it->second;
          return true;
        }

        return false;
      }

      KRYS_NODISCARD bool TryDecode(uint8 ch, Rune &out) const noexcept
      {
        if (ASCII::IsASCII(ch))
        {
          out = Rune(ch);
          return true;
        }

        const auto index = ch - ASCII::ExtendedASCIIStart;
        if (index >= _table.size() || _table[index] == 0u)
        {
          return false;
        }

        out = _table[index];
        return true;
      }
    };

  protected:
    LookupTable _lookupTable;

    SingleByteEncoding(const EncodingInfo &encodingInfo, const LookupTable &lookupTable) noexcept
        : Encoding(encodingInfo), _lookupTable(lookupTable)
    {
    }

  public:
    ~SingleByteEncoding() noexcept override = default;

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      uint8 ch {0u};
      const auto EncodeCodepoint = [&](Rune codepoint, bool wasInvalid) noexcept
      {
        if (!wasInvalid && _lookupTable.TryEncode(codepoint, ch))
        {
          out.push_back(static_cast<byte>(ch));
        }
        else
        {
          out.push_back(static_cast<byte>(ASCII::ReplacementCharacter));
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Rune codepoint {0u};
      for (byte b : bytes)
      {
        if (_lookupTable.TryDecode(static_cast<uint8>(b), codepoint))
        {
          Unicode::ToUTF8(codepoint, out);
        }
        else
        {
          Unicode::ToUTF8(Unicode::ReplacementCharacter, out);
        }
      }
    }
  };
}