#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_1_1998.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-1 encoding specification published in 1998.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_1_1998
      : public SingleByteLookupEncoding<basic_iso_8859_1_1998<TCodeUnit, TCodePoint>,
                                        &EncodingTable::ISO8859_1_1998IndexToCodePoint,
                                        &EncodingTable::ISO8859_1_1998CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "ISO-8859-1-1998"_s;
  };

  /// @brief The encoding that matches the ISO/IEC 8859-1 encoding specification published in 1998.
  constexpr inline basic_iso_8859_1_1998<char> iso_8859_1_1998 = {};
}
