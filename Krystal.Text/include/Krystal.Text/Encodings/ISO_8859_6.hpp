#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_6.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-6 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_6
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_6<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_6_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_6_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-6"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 14> Aliases = {
      "arabic"_s,    "asmo-708"_s,   "csiso88596e"_s,  "csiso88596i"_s,    "csisolatinarabic"_s,
      "ecma-114"_s,  "iso-8859-6"_s, "iso-8859-6-e"_s, "iso-8859-6-i"_s,   "iso-ir-127"_s,
      "iso8859-6"_s, "iso88596"_s,   "iso_8859-6"_s,   "iso_8859-6:1987"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-6 encoding specification.
  constexpr inline basic_iso_8859_6<char> iso_8859_6 = {};
}
