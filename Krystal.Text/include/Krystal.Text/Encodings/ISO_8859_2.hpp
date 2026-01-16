#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_2.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-2 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_2
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_2<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_2_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_2_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-2"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 9> Aliases = {
      "csisolatin2"_s, "iso-8859-2"_s,      "iso-ir-101"_s, "iso8859-2"_s, "iso88592"_s,
      "iso_8859-2"_s,  "iso_8859-2:1987"_s, "l2"_s,         "latin2"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-2 encoding specification.
  constexpr inline basic_iso_8859_2<char> iso_8859_2 = {};
}
