#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/MacRoman.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_roman
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_mac_roman<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::mac_roman_index_to_code_point,
          &::Krys::Text::EncodingTable::mac_roman_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"macintosh"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 4> Aliases = {
      "csmacintosh"_s, "mac"_s, "macintosh"_s, "x-mac-roman"_s};
  };

  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  constexpr inline basic_mac_roman<char> mac_roman = {};
}
