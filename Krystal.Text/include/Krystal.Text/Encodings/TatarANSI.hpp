#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/TatarANSI.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the TATAR encoding (ANSI variant, Based on Windows Code Page 1252.)
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ansi
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ansi<TCodeUnit, TCodePoint>, &EncodingTable::TatarANSIIndexToCodePoint,
          &EncodingTable::TatarANSICodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ASCIILiteral Name = "TATAR-ANSI";
  };

  /// @brief The encoding that matches the TATAR encoding (ANSI variant, Based on Windows Code Page 1252.)
  constexpr inline basic_tatar_ansi<char> tatar_ansi = {};
}
