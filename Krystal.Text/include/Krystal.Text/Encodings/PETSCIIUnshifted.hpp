#pragma once

#include "Krystal.Text/Encodings/EncodingTables/PETSCII.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for unshifted
  /// characters (when the SHIFT button was NOT pressed on a PET/CBM device).
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_petscii_unshifted
      : public Impl::SingleByteLookupEncoding<
          basic_petscii_unshifted<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::petscii_unshifted_index_to_code_point,
          &::Krys::Text::EncodingTable::petscii_unshifted_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_petscii_unshifted for ease of use.
  inline constexpr basic_petscii_unshifted<char> petscii_unshifted = {};
}
