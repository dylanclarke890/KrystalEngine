#pragma once

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
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1255.
  constexpr inline basic_windows_1255<char> windows_1255 = {};
}
