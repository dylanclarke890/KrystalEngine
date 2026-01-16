#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows874.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 874.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_874
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_874<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_874_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_874_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-874"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 6> Aliases = {"dos-874"_s, "iso-8859-11"_s, "iso8859-11"_s,
                                                                                    "iso885911"_s, "tis-620"_s, "windows-874"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 874.
  constexpr inline basic_windows_874<char> windows_874 = {};
}
