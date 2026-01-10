#pragma once

#include "Krystal.Text/Encodings/EncodingTables/PETSCII.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for shifted
  /// characters (when the SHIFT button was pressed on a PET/CBM device).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_petscii_shifted
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_petscii_shifted<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::petscii_shifted_index_to_code_point,
          &::Krys::Text::EncodingTable::petscii_shifted_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for shifted
  /// characters (when the SHIFT button was pressed on a PET/CBM device).
  constexpr inline basic_petscii_shifted<char> petscii_shifted = {};
}
