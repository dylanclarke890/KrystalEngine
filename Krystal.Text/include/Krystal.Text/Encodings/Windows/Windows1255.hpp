#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys::Text
{
  class Windows1255Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"windows-1255";
    static constexpr uint32 MIBenum = 2'255u;
    static constexpr uint32 WindowsCodePage = 1'255u;

    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, UnicodeCodepoint(0x20ACu)}, // EURO SIGN
      MapItem {0x82u, UnicodeCodepoint(0x201Au)}, // SINGLE LOW-9 QUOTATION MARK
      MapItem {0x83u, UnicodeCodepoint(0x0192u)}, // LATIN SMALL LETTER F WITH HOOK
      MapItem {0x84u, UnicodeCodepoint(0x201Eu)}, // DOUBLE LOW-9 QUOTATION MARK
      MapItem {0x85u, UnicodeCodepoint(0x2026u)}, // HORIZONTAL ELLIPSIS
      MapItem {0x86u, UnicodeCodepoint(0x2020u)}, // DAGGER
      MapItem {0x87u, UnicodeCodepoint(0x2021u)}, // DOUBLE DAGGER
      MapItem {0x88u, UnicodeCodepoint(0x02C6u)}, // MODIFIER LETTER CIRCUMFLEX ACCENT
      MapItem {0x89u, UnicodeCodepoint(0x2030u)}, // PER MILLE SIGN
      MapItem {0x8Bu, UnicodeCodepoint(0x2039u)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
      MapItem {0x91u, UnicodeCodepoint(0x2018u)}, // LEFT SINGLE QUOTATION MARK
      MapItem {0x92u, UnicodeCodepoint(0x2019u)}, // RIGHT SINGLE QUOTATION MARK
      MapItem {0x93u, UnicodeCodepoint(0x201Cu)}, // LEFT DOUBLE QUOTATION MARK
      MapItem {0x94u, UnicodeCodepoint(0x201Du)}, // RIGHT DOUBLE QUOTATION MARK
      MapItem {0x95u, UnicodeCodepoint(0x2022u)}, // BULLET
      MapItem {0x96u, UnicodeCodepoint(0x2013u)}, // EN DASH
      MapItem {0x97u, UnicodeCodepoint(0x2014u)}, // EM DASH
      MapItem {0x98u, UnicodeCodepoint(0x02DCu)}, // SMALL TILDE
      MapItem {0x99u, UnicodeCodepoint(0x2122u)}, // TRADE MARK SIGN
      MapItem {0x9Bu, UnicodeCodepoint(0x203Au)}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
      MapItem {0xA0u, UnicodeCodepoint(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, UnicodeCodepoint(0x00A1u)}, // INVERTED EXCLAMATION MARK
      MapItem {0xA2u, UnicodeCodepoint(0x00A2u)}, // CENT SIGN
      MapItem {0xA3u, UnicodeCodepoint(0x00A3u)}, // POUND SIGN
      MapItem {0xA4u, UnicodeCodepoint(0x20AAu)}, // NEW SHEQEL SIGN
      MapItem {0xA5u, UnicodeCodepoint(0x00A5u)}, // YEN SIGN
      MapItem {0xA6u, UnicodeCodepoint(0x00A6u)}, // BROKEN BAR
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, UnicodeCodepoint(0x00A9u)}, // COPYRIGHT SIGN
      MapItem {0xAAu, UnicodeCodepoint(0x00D7u)}, // MULTIPLICATION SIGN
      MapItem {0xABu, UnicodeCodepoint(0x00ABu)}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, UnicodeCodepoint(0x00ACu)}, // NOT SIGN
      MapItem {0xADu, UnicodeCodepoint(0x00ADu)}, // SOFT HYPHEN
      MapItem {0xAEu, UnicodeCodepoint(0x00AEu)}, // REGISTERED SIGN
      MapItem {0xAFu, UnicodeCodepoint(0x00AFu)}, // MACRON
      MapItem {0xB0u, UnicodeCodepoint(0x00B0u)}, // DEGREE SIGN
      MapItem {0xB1u, UnicodeCodepoint(0x00B1u)}, // PLUS-MINUS SIGN
      MapItem {0xB2u, UnicodeCodepoint(0x00B2u)}, // SUPERSCRIPT TWO
      MapItem {0xB3u, UnicodeCodepoint(0x00B3u)}, // SUPERSCRIPT THREE
      MapItem {0xB4u, UnicodeCodepoint(0x00B4u)}, // ACUTE ACCENT
      MapItem {0xB5u, UnicodeCodepoint(0x00B5u)}, // MICRO SIGN
      MapItem {0xB6u, UnicodeCodepoint(0x00B6u)}, // PILCROW SIGN
      MapItem {0xB7u, UnicodeCodepoint(0x00B7u)}, // MIDDLE DOT
      MapItem {0xB8u, UnicodeCodepoint(0x00B8u)}, // CEDILLA
      MapItem {0xB9u, UnicodeCodepoint(0x00B9u)}, // SUPERSCRIPT ONE
      MapItem {0xBAu, UnicodeCodepoint(0x00F7u)}, // DIVISION SIGN
      MapItem {0xBBu, UnicodeCodepoint(0x00BBu)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, UnicodeCodepoint(0x00BCu)}, // VULGAR FRACTION ONE QUARTER
      MapItem {0xBDu, UnicodeCodepoint(0x00BDu)}, // VULGAR FRACTION ONE HALF
      MapItem {0xBEu, UnicodeCodepoint(0x00BEu)}, // VULGAR FRACTION THREE QUARTERS
      MapItem {0xBFu, UnicodeCodepoint(0x00BFu)}, // INVERTED QUESTION MARK
      MapItem {0xC0u, UnicodeCodepoint(0x05B0u)}, // HEBREW POINT SHEVA
      MapItem {0xC1u, UnicodeCodepoint(0x05B1u)}, // HEBREW POINT HATAF SEGOL
      MapItem {0xC2u, UnicodeCodepoint(0x05B2u)}, // HEBREW POINT HATAF PATAH
      MapItem {0xC3u, UnicodeCodepoint(0x05B3u)}, // HEBREW POINT HATAF QAMATS
      MapItem {0xC4u, UnicodeCodepoint(0x05B4u)}, // HEBREW POINT HIRIQ
      MapItem {0xC5u, UnicodeCodepoint(0x05B5u)}, // HEBREW POINT TSERE
      MapItem {0xC6u, UnicodeCodepoint(0x05B6u)}, // HEBREW POINT SEGOL
      MapItem {0xC7u, UnicodeCodepoint(0x05B7u)}, // HEBREW POINT PATAH
      MapItem {0xC8u, UnicodeCodepoint(0x05B8u)}, // HEBREW POINT QAMATS
      MapItem {0xC9u, UnicodeCodepoint(0x05B9u)}, // HEBREW POINT HOLAM
      MapItem {0xCBu, UnicodeCodepoint(0x05BBu)}, // HEBREW POINT QUBUTS
      MapItem {0xCCu, UnicodeCodepoint(0x05BCu)}, // HEBREW POINT DAGESH OR MAPIQ
      MapItem {0xCDu, UnicodeCodepoint(0x05BDu)}, // HEBREW POINT METEG
      MapItem {0xCEu, UnicodeCodepoint(0x05BEu)}, // HEBREW PUNCTUATION MAQAF
      MapItem {0xCFu, UnicodeCodepoint(0x05BFu)}, // HEBREW POINT RAFE
      MapItem {0xD0u, UnicodeCodepoint(0x05C0u)}, // HEBREW PUNCTUATION PASEQ
      MapItem {0xD1u, UnicodeCodepoint(0x05C1u)}, // HEBREW POINT SHIN DOT
      MapItem {0xD2u, UnicodeCodepoint(0x05C2u)}, // HEBREW POINT SIN DOT
      MapItem {0xD3u, UnicodeCodepoint(0x05C3u)}, // HEBREW PUNCTUATION SOF PASUQ
      MapItem {0xD4u, UnicodeCodepoint(0x05F0u)}, // HEBREW LIGATURE YIDDISH DOUBLE VAV
      MapItem {0xD5u, UnicodeCodepoint(0x05F1u)}, // HEBREW LIGATURE YIDDISH VAV YOD
      MapItem {0xD6u, UnicodeCodepoint(0x05F2u)}, // HEBREW LIGATURE YIDDISH DOUBLE YOD
      MapItem {0xD7u, UnicodeCodepoint(0x05F3u)}, // HEBREW PUNCTUATION GERESH
      MapItem {0xD8u, UnicodeCodepoint(0x05F4u)}, // HEBREW PUNCTUATION GERSHAYIM
      MapItem {0xE0u, UnicodeCodepoint(0x05D0u)}, // HEBREW LETTER ALEF
      MapItem {0xE1u, UnicodeCodepoint(0x05D1u)}, // HEBREW LETTER BET
      MapItem {0xE2u, UnicodeCodepoint(0x05D2u)}, // HEBREW LETTER GIMEL
      MapItem {0xE3u, UnicodeCodepoint(0x05D3u)}, // HEBREW LETTER DALET
      MapItem {0xE4u, UnicodeCodepoint(0x05D4u)}, // HEBREW LETTER HE
      MapItem {0xE5u, UnicodeCodepoint(0x05D5u)}, // HEBREW LETTER VAV
      MapItem {0xE6u, UnicodeCodepoint(0x05D6u)}, // HEBREW LETTER ZAYIN
      MapItem {0xE7u, UnicodeCodepoint(0x05D7u)}, // HEBREW LETTER HET
      MapItem {0xE8u, UnicodeCodepoint(0x05D8u)}, // HEBREW LETTER TET
      MapItem {0xE9u, UnicodeCodepoint(0x05D9u)}, // HEBREW LETTER YOD
      MapItem {0xEAu, UnicodeCodepoint(0x05DAu)}, // HEBREW LETTER FINAL KAF
      MapItem {0xEBu, UnicodeCodepoint(0x05DBu)}, // HEBREW LETTER KAF
      MapItem {0xECu, UnicodeCodepoint(0x05DCu)}, // HEBREW LETTER LAMED
      MapItem {0xEDu, UnicodeCodepoint(0x05DDu)}, // HEBREW LETTER FINAL MEM
      MapItem {0xEEu, UnicodeCodepoint(0x05DEu)}, // HEBREW LETTER MEM
      MapItem {0xEFu, UnicodeCodepoint(0x05DFu)}, // HEBREW LETTER FINAL NUN
      MapItem {0xF0u, UnicodeCodepoint(0x05E0u)}, // HEBREW LETTER NUN
      MapItem {0xF1u, UnicodeCodepoint(0x05E1u)}, // HEBREW LETTER SAMEKH
      MapItem {0xF2u, UnicodeCodepoint(0x05E2u)}, // HEBREW LETTER AYIN
      MapItem {0xF3u, UnicodeCodepoint(0x05E3u)}, // HEBREW LETTER FINAL PE
      MapItem {0xF4u, UnicodeCodepoint(0x05E4u)}, // HEBREW LETTER PE
      MapItem {0xF5u, UnicodeCodepoint(0x05E5u)}, // HEBREW LETTER FINAL TSADI
      MapItem {0xF6u, UnicodeCodepoint(0x05E6u)}, // HEBREW LETTER TSADI
      MapItem {0xF7u, UnicodeCodepoint(0x05E7u)}, // HEBREW LETTER QOF
      MapItem {0xF8u, UnicodeCodepoint(0x05E8u)}, // HEBREW LETTER RESH
      MapItem {0xF9u, UnicodeCodepoint(0x05E9u)}, // HEBREW LETTER SHIN
      MapItem {0xFAu, UnicodeCodepoint(0x05EAu)}, // HEBREW LETTER TAV
      MapItem {0xFDu, UnicodeCodepoint(0x200Eu)}, // LEFT-TO-RIGHT MARK
      MapItem {0xFEu, UnicodeCodepoint(0x200Fu)}, // RIGHT-TO-LEFT MARK
    };

  public:
    Windows1255Encoding() noexcept : SingleByteEncoding({ Name, MIBenum, WindowsCodePage }, LookupTable(LookupMapping))
    {
    }

    ~Windows1255Encoding() noexcept override = default;
  };
}