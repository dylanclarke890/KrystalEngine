#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/IBM856Hebrew.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Hebrew encoding that matches IBM's Codepage 856.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_856_hebrew
      : public SingleByteLookupEncoding<basic_ibm_856_hebrew<TCodeUnit, TCodePoint>,
                                        &EncodingTable::IMB856HebrewIndexToCodePoint,
                                        &EncodingTable::IMB856HebrewCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ASCIILiteral Name = "IBM-856";
  };

  /// @brief An instance of basic_ibm_856_hebrew for ease of use.
  constexpr inline basic_ibm_856_hebrew<char> ibm_856_hebrew = {};
}
