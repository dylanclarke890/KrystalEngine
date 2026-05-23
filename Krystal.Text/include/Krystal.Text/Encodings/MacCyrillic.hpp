#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/MacCyrillic.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_cyrillic
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_mac_cyrillic<TCodeUnit, TCodePoint>, &EncodingTable::MacCyrillicIndexToCodePoint,
          &EncodingTable::MacCyrillicCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"x-mac-cyrillic"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 2> Aliases = {"x-mac-cyrillic"_s,
                                                                                    "x-mac-ukranian"_s};
  };

  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  constexpr inline basic_mac_cyrillic<char> mac_cyrillic = {};
}
