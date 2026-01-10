#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1252.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1252 (AKA "Latin-1").
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1252
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1252<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1252_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1252_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1252 (AKA "Latin-1").
  constexpr inline basic_windows_1252<char> windows_1252 = {};
}
