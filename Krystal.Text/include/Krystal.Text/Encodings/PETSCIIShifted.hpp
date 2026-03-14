#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/PETSCII.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for shifted
  /// characters (when the SHIFT button was pressed on a PET/CBM device).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_petscii_shifted
      : public SingleByteLookupEncoding<basic_petscii_shifted<TCodeUnit, TCodePoint>,
                                        &EncodingTable::PETSCIIShiftedIndexToCodePoint,
                                        &EncodingTable::PETSCIIShiftedCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "PETSCII"_s;
  };

  /// @brief The encoding that matches the PETSCII (CBM ASCII) encoding specification, for shifted
  /// characters (when the SHIFT button was pressed on a PET/CBM device).
  constexpr inline basic_petscii_shifted<char> petscii_shifted = {};
}
