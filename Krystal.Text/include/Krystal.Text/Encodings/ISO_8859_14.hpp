#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_14.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-14 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_14
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_14<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_14IndexToCodePoint,
          &EncodingTable::ISO_8859_14CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-14"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {
      "iso-8859-14"_s, "iso8859-14"_s, "iso885914"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-14 encoding specification.
  constexpr inline basic_iso_8859_14<char> iso_8859_14 = {};
}
