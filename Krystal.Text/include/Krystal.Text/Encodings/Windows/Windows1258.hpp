#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class Windows1258Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"windows-1258";
    static constexpr uint32 MIBenum = 2'258u;
    static constexpr uint32 WindowsCodePage = 1'258u;

    static constexpr Mapping LookupMapping = {
      MapItem {0x82u, UnicodeCodepoint(0x201Au)}, // SINGLE LOW-9 QUOTATION MARK
      MapItem {0x83u, UnicodeCodepoint(0x0192u)}, // LATIN SMALL LETTER F WITH HOOK
      MapItem {0x84u, UnicodeCodepoint(0x201Eu)}, // DOUBLE LOW-9 QUOTATION MARK
      MapItem {0x85u, UnicodeCodepoint(0x2026u)}, // HORIZONTAL ELLIPSIS
      MapItem {0x86u, UnicodeCodepoint(0x2020u)}, // DAGGER
      MapItem {0x87u, UnicodeCodepoint(0x2021u)}, // DOUBLE DAGGER
      MapItem {0x88u, UnicodeCodepoint(0x02C6u)}, // MODIFIER LETTER CIRCUMFLEX ACCENT
      MapItem {0x89u, UnicodeCodepoint(0x2030u)}, // PER MILLE SIGN
      MapItem {0x8Bu, UnicodeCodepoint(0x2039u)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
      MapItem {0x8Cu, UnicodeCodepoint(0x0152u)}, // LATIN CAPITAL LIGATURE OE
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
      MapItem {0x9Cu, UnicodeCodepoint(0x0153u)}, // LATIN SMALL LIGATURE OE
      MapItem {0x9Fu, UnicodeCodepoint(0x0178u)}, // LATIN CAPITAL LETTER Y WITH DIAERESIS
      MapItem {0xA0u, UnicodeCodepoint(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, UnicodeCodepoint(0x00A1u)}, // INVERTED EXCLAMATION MARK
      MapItem {0xA2u, UnicodeCodepoint(0x00A2u)}, // CENT SIGN
      MapItem {0xA3u, UnicodeCodepoint(0x00A3u)}, // POUND SIGN
      MapItem {0xA4u, UnicodeCodepoint(0x00A4u)}, // CURRENCY SIGN
      MapItem {0xA5u, UnicodeCodepoint(0x00A5u)}, // YEN SIGN
      MapItem {0xA6u, UnicodeCodepoint(0x00A6u)}, // BROKEN BAR
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, UnicodeCodepoint(0x00A9u)}, // COPYRIGHT SIGN
      MapItem {0xAAu, UnicodeCodepoint(0x00AAu)}, // FEMININE ORDINAL INDICATOR
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
      MapItem {0xBAu, UnicodeCodepoint(0x00BAu)}, // MASCULINE ORDINAL INDICATOR
      MapItem {0xBBu, UnicodeCodepoint(0x00BBu)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, UnicodeCodepoint(0x00BCu)}, // VULGAR FRACTION ONE QUARTER
      MapItem {0xBDu, UnicodeCodepoint(0x00BDu)}, // VULGAR FRACTION ONE HALF
      MapItem {0xBEu, UnicodeCodepoint(0x00BEu)}, // VULGAR FRACTION THREE QUARTERS
      MapItem {0xBFu, UnicodeCodepoint(0x00BFu)}, // INVERTED QUESTION MARK
      MapItem {0xC0u, UnicodeCodepoint(0x00C0u)}, // LATIN CAPITAL LETTER A WITH GRAVE
      MapItem {0xC1u, UnicodeCodepoint(0x00C1u)}, // LATIN CAPITAL LETTER A WITH ACUTE
      MapItem {0xC2u, UnicodeCodepoint(0x00C2u)}, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
      MapItem {0xC3u, UnicodeCodepoint(0x0102u)}, // LATIN CAPITAL LETTER A WITH BREVE
      MapItem {0xC4u, UnicodeCodepoint(0x00C4u)}, // LATIN CAPITAL LETTER A WITH DIAERESIS
      MapItem {0xC5u, UnicodeCodepoint(0x00C5u)}, // LATIN CAPITAL LETTER A WITH RING ABOVE
      MapItem {0xC6u, UnicodeCodepoint(0x00C6u)}, // LATIN CAPITAL LETTER AE
      MapItem {0xC7u, UnicodeCodepoint(0x00C7u)}, // LATIN CAPITAL LETTER C WITH CEDILLA
      MapItem {0xC8u, UnicodeCodepoint(0x00C8u)}, // LATIN CAPITAL LETTER E WITH GRAVE
      MapItem {0xC9u, UnicodeCodepoint(0x00C9u)}, // LATIN CAPITAL LETTER E WITH ACUTE
      MapItem {0xCAu, UnicodeCodepoint(0x00CAu)}, // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
      MapItem {0xCBu, UnicodeCodepoint(0x00CBu)}, // LATIN CAPITAL LETTER E WITH DIAERESIS
      MapItem {0xCCu, UnicodeCodepoint(0x0300u)}, // COMBINING GRAVE ACCENT
      MapItem {0xCDu, UnicodeCodepoint(0x00CDu)}, // LATIN CAPITAL LETTER I WITH ACUTE
      MapItem {0xCEu, UnicodeCodepoint(0x00CEu)}, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
      MapItem {0xCFu, UnicodeCodepoint(0x00CFu)}, // LATIN CAPITAL LETTER I WITH DIAERESIS
      MapItem {0xD0u, UnicodeCodepoint(0x0110u)}, // LATIN CAPITAL LETTER D WITH STROKE
      MapItem {0xD1u, UnicodeCodepoint(0x00D1u)}, // LATIN CAPITAL LETTER N WITH TILDE
      MapItem {0xD2u, UnicodeCodepoint(0x0309u)}, // COMBINING HOOK ABOVE
      MapItem {0xD3u, UnicodeCodepoint(0x00D3u)}, // LATIN CAPITAL LETTER O WITH ACUTE
      MapItem {0xD4u, UnicodeCodepoint(0x00D4u)}, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
      MapItem {0xD5u, UnicodeCodepoint(0x01A0u)}, // LATIN CAPITAL LETTER O WITH HORN
      MapItem {0xD6u, UnicodeCodepoint(0x00D6u)}, // LATIN CAPITAL LETTER O WITH DIAERESIS
      MapItem {0xD7u, UnicodeCodepoint(0x00D7u)}, // MULTIPLICATION SIGN
      MapItem {0xD8u, UnicodeCodepoint(0x00D8u)}, // LATIN CAPITAL LETTER O WITH STROKE
      MapItem {0xD9u, UnicodeCodepoint(0x00D9u)}, // LATIN CAPITAL LETTER U WITH GRAVE
      MapItem {0xDAu, UnicodeCodepoint(0x00DAu)}, // LATIN CAPITAL LETTER U WITH ACUTE
      MapItem {0xDBu, UnicodeCodepoint(0x00DBu)}, // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
      MapItem {0xDCu, UnicodeCodepoint(0x00DCu)}, // LATIN CAPITAL LETTER U WITH DIAERESIS
      MapItem {0xDDu, UnicodeCodepoint(0x01AFu)}, // LATIN CAPITAL LETTER U WITH HORN
      MapItem {0xDEu, UnicodeCodepoint(0x0303u)}, // COMBINING TILDE
      MapItem {0xDFu, UnicodeCodepoint(0x00DFu)}, // LATIN SMALL LETTER SHARP S
      MapItem {0xE0u, UnicodeCodepoint(0x00E0u)}, // LATIN SMALL LETTER A WITH GRAVE
      MapItem {0xE1u, UnicodeCodepoint(0x00E1u)}, // LATIN SMALL LETTER A WITH ACUTE
      MapItem {0xE2u, UnicodeCodepoint(0x00E2u)}, // LATIN SMALL LETTER A WITH CIRCUMFLEX
      MapItem {0xE3u, UnicodeCodepoint(0x0103u)}, // LATIN SMALL LETTER A WITH BREVE
      MapItem {0xE4u, UnicodeCodepoint(0x00E4u)}, // LATIN SMALL LETTER A WITH DIAERESIS
      MapItem {0xE5u, UnicodeCodepoint(0x00E5u)}, // LATIN SMALL LETTER A WITH RING ABOVE
      MapItem {0xE6u, UnicodeCodepoint(0x00E6u)}, // LATIN SMALL LETTER AE
      MapItem {0xE7u, UnicodeCodepoint(0x00E7u)}, // LATIN SMALL LETTER C WITH CEDILLA
      MapItem {0xE8u, UnicodeCodepoint(0x00E8u)}, // LATIN SMALL LETTER E WITH GRAVE
      MapItem {0xE9u, UnicodeCodepoint(0x00E9u)}, // LATIN SMALL LETTER E WITH ACUTE
      MapItem {0xEAu, UnicodeCodepoint(0x00EAu)}, // LATIN SMALL LETTER E WITH CIRCUMFLEX
      MapItem {0xEBu, UnicodeCodepoint(0x00EBu)}, // LATIN SMALL LETTER E WITH DIAERESIS
      MapItem {0xECu, UnicodeCodepoint(0x0301u)}, // COMBINING ACUTE ACCENT
      MapItem {0xEDu, UnicodeCodepoint(0x00EDu)}, // LATIN SMALL LETTER I WITH ACUTE
      MapItem {0xEEu, UnicodeCodepoint(0x00EEu)}, // LATIN SMALL LETTER I WITH CIRCUMFLEX
      MapItem {0xEFu, UnicodeCodepoint(0x00EFu)}, // LATIN SMALL LETTER I WITH DIAERESIS
      MapItem {0xF0u, UnicodeCodepoint(0x0111u)}, // LATIN SMALL LETTER D WITH STROKE
      MapItem {0xF1u, UnicodeCodepoint(0x00F1u)}, // LATIN SMALL LETTER N WITH TILDE
      MapItem {0xF2u, UnicodeCodepoint(0x0323u)}, // COMBINING DOT BELOW
      MapItem {0xF3u, UnicodeCodepoint(0x00F3u)}, // LATIN SMALL LETTER O WITH ACUTE
      MapItem {0xF4u, UnicodeCodepoint(0x00F4u)}, // LATIN SMALL LETTER O WITH CIRCUMFLEX
      MapItem {0xF5u, UnicodeCodepoint(0x01A1u)}, // LATIN SMALL LETTER O WITH HORN
      MapItem {0xF6u, UnicodeCodepoint(0x00F6u)}, // LATIN SMALL LETTER O WITH DIAERESIS
      MapItem {0xF7u, UnicodeCodepoint(0x00F7u)}, // DIVISION SIGN
      MapItem {0xF8u, UnicodeCodepoint(0x00F8u)}, // LATIN SMALL LETTER O WITH STROKE
      MapItem {0xF9u, UnicodeCodepoint(0x00F9u)}, // LATIN SMALL LETTER U WITH GRAVE
      MapItem {0xFAu, UnicodeCodepoint(0x00FAu)}, // LATIN SMALL LETTER U WITH ACUTE
      MapItem {0xFBu, UnicodeCodepoint(0x00FBu)}, // LATIN SMALL LETTER U WITH CIRCUMFLEX
      MapItem {0xFCu, UnicodeCodepoint(0x00FCu)}, // LATIN SMALL LETTER U WITH DIAERESIS
      MapItem {0xFDu, UnicodeCodepoint(0x01B0u)}, // LATIN SMALL LETTER U WITH HORN
      MapItem {0xFEu, UnicodeCodepoint(0x20ABu)}, // DONG SIGN
      MapItem {0xFFu, UnicodeCodepoint(0x00FFu)}, // LATIN SMALL LETTER Y WITH DIAERESIS
    };

  public:
    Windows1258Encoding() noexcept : SingleByteEncoding({ Name, MIBenum, WindowsCodePage }, LookupTable(LookupMapping))
    {
    }

    ~Windows1258Encoding() noexcept override = default;
  };
}