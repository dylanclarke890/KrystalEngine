#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1255.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1255.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1255
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1255<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1255_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1255_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1255"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1255"_s,
                                                                                    "cp1255"_s, "x-cp1255"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1255.
  constexpr inline basic_windows_1255<char> windows_1255 = {};
}
