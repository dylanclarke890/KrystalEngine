#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_1_1985.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The encoding that matches ISO/IEC 8859-1 specification published in 1985.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_1_1985
      : public SingleByteLookupEncoding<basic_iso_8859_1_1985<TCodeUnit, TCodePoint>,
                                        &EncodingTable::ISO8859_1_1985IndexToCodePoint,
                                        &EncodingTable::ISO8859_1_1985CodePointToIndex, TCodeUnit, TCodePoint>
  {
  private:
    static_assert(((sizeof(TCodePoint) * CHAR_BIT) > 15),
                  "The code point type for ISO 8859-1 (1985) must be at least 16 bits wide");

  public:
    constexpr static ::Krys::Text::ASCIILiteral Name = "ISO-8859-1-1985"_s;
  };

  /// @brief The encoding that matches ISO/IEC 8859-1 specification published in 1985.
  constexpr inline basic_iso_8859_1_1985<char> iso_8859_1_1985 = {};
}
