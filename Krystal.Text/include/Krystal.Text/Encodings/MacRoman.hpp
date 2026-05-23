#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/MacRoman.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_roman : public SingleASCIIByteHighBitLookupEncoding<
                            basic_mac_roman<TCodeUnit, TCodePoint>, &EncodingTable::MacRomanIndexToCodePoint,
                            &EncodingTable::MacRomanCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"macintosh"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 4> Aliases = {
      "csmacintosh"_s, "mac"_s, "macintosh"_s, "x-mac-roman"_s};
  };

  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  constexpr inline basic_mac_roman<char> mac_roman = {};
}
