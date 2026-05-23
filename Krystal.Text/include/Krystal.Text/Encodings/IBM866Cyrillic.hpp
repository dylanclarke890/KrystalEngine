#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/IBM866Cyrillic.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Cyrillic encoding that matches IBM's Codepage 866.
  ///
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_866_cyrillic
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_ibm_866_cyrillic<TCodeUnit, TCodePoint>, &EncodingTable::IMB866CyrillicIndexToCodePoint,
          &EncodingTable::IMB866CyrillicCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ASCIILiteral Name = "IBM866"_s;
    constexpr static inline Array<ASCIILiteral, 4> Aliases = {"cp866"_s, "ibm866"_s, "866"_s, "csibm866"_s};
  };

  /// @brief An instance of basic_ibm_866_cyrillic for ease of use.
  constexpr inline basic_ibm_866_cyrillic<char> ibm_866_cyrillic = {};
}
