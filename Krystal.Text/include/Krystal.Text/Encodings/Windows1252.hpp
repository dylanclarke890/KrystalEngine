#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1252.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1252 (AKA "Latin-1").
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1252
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1252<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1252_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1252_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1252"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 17> Aliases = {
      "ansi_x3.4-1968"_s, "ascii"_s,           "cp1252"_s,     "cp819"_s,     "csisolatin1"_s,
      "ibm819"_s,         "iso-8859-1"_s,      "iso-ir-100"_s, "iso8859-1"_s, "iso88591"_s,
      "iso_8859-1"_s,     "iso_8859-1:1987"_s, "l1"_s,         "latin1"_s,    "us-ascii"_s,
      "windows-1252"_s,   "x-cp1252"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1252 (AKA "Latin-1").
  constexpr inline basic_windows_1252<char> windows_1252 = {};
}
