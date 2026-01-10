#pragma once

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
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 874.
  constexpr inline basic_windows_874<char> windows_874 = {};
}
