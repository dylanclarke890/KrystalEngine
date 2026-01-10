#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1256.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1256.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1256
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1256<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1256_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1256_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1256.
  constexpr inline basic_windows_1256<char> windows_1256 = {};
}
