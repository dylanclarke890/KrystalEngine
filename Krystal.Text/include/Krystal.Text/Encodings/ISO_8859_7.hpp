#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_7.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-7 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_7
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_7<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_7IndexToCodePoint,
          &EncodingTable::ISO_8859_7CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "ISO-8859-7"_s;
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 13> Aliases = {
      "csisolatingreek"_s, "ecma-118"_s,   "elot_928"_s,        "greek"_s,
      "greek8"_s,          "iso-8859-7"_s, "iso-ir-126"_s,      "iso8859-7"_s,
      "iso88597"_s,        "iso_8859-7"_s, "iso_8859-7:1987"_s, "sun_eu_greek"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-7 encoding specification.
  constexpr inline basic_iso_8859_7<char> iso_8859_7 = {};
}
