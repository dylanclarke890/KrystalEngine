#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_10.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-10 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_10
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_10<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_10IndexToCodePoint,
          &EncodingTable::ISO_8859_10CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "ISO-8859-10"_s;
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 7> Aliases = {
      "csisolatin6"_s, "iso-8859-10"_s, "iso-ir-157"_s, "iso8859-10"_s, "iso885910"_s, "l6"_s, "latin6"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-10 encoding specification.
  constexpr inline basic_iso_8859_10<char> iso_8859_10 = {};
}
