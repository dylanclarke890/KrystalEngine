#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1257.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1257.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1257
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1257<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1257_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1257_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1257.
  constexpr inline basic_windows_1257<char> windows_1257 = {};
}
