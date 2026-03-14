#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_13.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-15 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_13
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_13<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_13IndexToCodePoint,
          &EncodingTable::ISO_8859_13CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-13"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {
      "iso-8859-13"_s, "iso8859-13"_s, "iso885913"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-15 encoding specification.
  constexpr inline basic_iso_8859_13<char> iso_8859_13 = {};
}
