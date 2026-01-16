#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/PETSCII.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for unshifted
  /// characters (when the SHIFT button was NOT pressed on a PET/CBM device).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_petscii_unshifted
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_petscii_unshifted<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::petscii_unshifted_index_to_code_point,
          &::Krys::Text::EncodingTable::petscii_unshifted_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "PETSCII"_s;
  };

  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for unshifted
  /// characters (when the SHIFT button was NOT pressed on a PET/CBM device).
  constexpr inline basic_petscii_unshifted<char> petscii_unshifted = {};
}
