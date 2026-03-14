#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/TatarASCII.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the TATAR encoding specification ("ASCII" variant, IBM Code Page 866).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ascii
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ascii<TCodeUnit, TCodePoint>, &EncodingTable::TatarASCIIIndexToCodePoint,
          &EncodingTable::TatarASCIICodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ASCIILiteral Name = "TATAR-ASCII";
  };

  /// @brief The encoding that matches the TATAR encoding specification ("ASCII" variant, IBM Code Page 866).
  constexpr inline basic_tatar_ascii<char> tatar_ascii = {};
}
