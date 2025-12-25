#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Pair.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Encodings/EncodingConstants.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::Text
{
  class SingleByteEncoding : public Encoding
  {
    constexpr static uint8 ASCIIOffset = Unicode::MaxASCIIValue + 1u;

  protected:
    class LookupTable
    {
    public:
      using ASCIIToCodepointTable = Array<UnicodeCodepoint, 128u>;
      using CodepointToASCIITable = Map<UnicodeCodepoint, uint8, StronglyTypedNumberHasher<UnicodeCodepoint>>;

      using MapItem = Pair<uint8, UnicodeCodepoint>;
      using Mapping = Array<MapItem, 128u>;

    private:
      ASCIIToCodepointTable _table {};
      CodepointToASCIITable _reverseTable {};

    public:
      explicit LookupTable(const Mapping &mappings) noexcept
      {
        for (const auto [ch, codepoint] : mappings)
        {
          const auto index = ch - ASCIIOffset;
          _table[index] = codepoint;
          _reverseTable[codepoint] = ch;
        }
      }

      NO_DISCARD bool TryEncode(UnicodeCodepoint codepoint, uint8 &out) const noexcept
      {
        if (Unicode::IsASCIICharacter(codepoint))
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

      NO_DISCARD bool TryDecode(uint8 ch, UnicodeCodepoint &out) const noexcept
      {
        if (Unicode::IsASCIICharacter(ch))
        {
          out = UnicodeCodepoint(ch);
          return true;
        }

        const auto index = ch - ASCIIOffset;
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
        : Encoding(encodingInfo, EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF)),
          _lookupTable(lookupTable)
    {
    }

    virtual ~SingleByteEncoding() noexcept override = default;

    NO_DISCARD virtual bool IsSingleByte() const noexcept override final
    {
      return true;
    }

    virtual void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size());

      uint8 ch {0u};
      const auto EncodeCodepoint = [&](UnicodeCodepoint codepoint, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
          return;
        }

        if (_lookupTable.TryEncode(codepoint, ch))
        {
          out.push_back(static_cast<byte>(ch));
        }
        else
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    virtual void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      UnicodeCodepoint codepoint {0u};
      for (byte b : bytes)
      {
        if (_lookupTable.TryDecode(static_cast<uint8>(b), codepoint))
        {
          Unicode::CodepointToUTF8(codepoint, out);
        }
        else
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
      }
    }
  };
}