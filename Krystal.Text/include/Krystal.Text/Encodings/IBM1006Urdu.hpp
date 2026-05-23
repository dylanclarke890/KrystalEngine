#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/IBM1006Urdu.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Urdu encoding that matches IBM's Codepage 1006.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_1006_urdu
      : public SingleByteLookupEncoding<
          basic_ibm_1006_urdu<TCodeUnit, TCodePoint>, &EncodingTable::IBM1006UrduIndexToCodePoint,
          &EncodingTable::IBM1006UrduCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "IBM-1006_Urdu"_s;
  };

  /// @brief The Urdu encoding that matches IBM's Codepage 1006.
  constexpr inline basic_ibm_1006_urdu<char> ibm_1006_urdu = {};
}
