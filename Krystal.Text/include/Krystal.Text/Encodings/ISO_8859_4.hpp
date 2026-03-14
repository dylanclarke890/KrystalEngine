#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_4.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-4 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_4
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_4<TCodeUnit, TCodePoint>, &EncodingTable::iso_8859_4_index_to_code_point,
          &EncodingTable::ISO_8859_4CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-4"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 9> Aliases = {
      "csisolatin4"_s, "iso-8859-4"_s,      "iso-ir-110"_s, "iso8859-4"_s, "iso88594"_s,
      "iso_8859-4"_s,  "iso_8859-4:1988"_s, "l4"_s,         "latin4"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-4 encoding specification.
  constexpr inline basic_iso_8859_4<char> iso_8859_4 = {};
}
