#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1258.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1258.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1258
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1258<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1258_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1258_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1258.
  constexpr inline basic_windows_1258<char> windows_1258 = {};
}
