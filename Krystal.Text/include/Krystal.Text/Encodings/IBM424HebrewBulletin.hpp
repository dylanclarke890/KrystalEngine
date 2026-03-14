#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/IBM424HebrewBulletin.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The Hebrew encoding that matches IBM's Codepage 424.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_424_hebrew_bulletin
      : public SingleByteLookupEncoding<
          basic_ibm_424_hebrew_bulletin<TCodeUnit, TCodePoint>,
          &EncodingTable::IMB424HebrewBulletinIndexToCodePoint,
          &EncodingTable::IMB424HebrewBulletinCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "IBM-424";
  };

  /// @brief The Hebrew encoding that matches IBM's Codepage 424.
  constexpr inline basic_ibm_424_hebrew_bulletin<char> ibm_424_hebrew_bulletin = {};
}
