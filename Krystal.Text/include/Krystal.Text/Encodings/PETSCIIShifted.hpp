#pragma once

#include "Krystal.Text/Encodings/EncodingTables/PETSCII.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for shifted
  /// characters (when the SHIFT button was pressed on a PET/CBM device).
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_petscii_shifted
      : public Impl::SingleByteLookupEncoding<
          basic_petscii_shifted<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::petscii_shifted_index_to_code_point,
          &::Krys::Text::EncodingTable::petscii_shifted_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_petscii_shifted for ease of use.
  constexpr inline basic_petscii_shifted<char> petscii_shifted = {};
}
