#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class Windows1250Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"windows-1250",
        .Aliases = {u8"windows-1250"},
        .MIBenum = MIBenum {2'250u},
        .Win32CodePage = Win32CodePage {1'250u},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, Rune(0x20ACu)}, // EURO SIGN
      MapItem {0x82u, Rune(0x201Au)}, // SINGLE LOW-9 QUOTATION MARK
      MapItem {0x84u, Rune(0x201Eu)}, // DOUBLE LOW-9 QUOTATION MARK
      MapItem {0x85u, Rune(0x2026u)}, // HORIZONTAL ELLIPSIS
      MapItem {0x86u, Rune(0x2020u)}, // DAGGER
      MapItem {0x87u, Rune(0x2021u)}, // DOUBLE DAGGER
      MapItem {0x89u, Rune(0x2030u)}, // PER MILLE SIGN
      MapItem {0x8Au, Rune(0x0160u)}, // LATIN CAPITAL LETTER S WITH CARON
      MapItem {0x8Bu, Rune(0x2039u)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
      MapItem {0x8Cu, Rune(0x015Au)}, // LATIN CAPITAL LETTER S WITH ACUTE
      MapItem {0x8Du, Rune(0x0164u)}, // LATIN CAPITAL LETTER T WITH CARON
      MapItem {0x8Eu, Rune(0x017Du)}, // LATIN CAPITAL LETTER Z WITH CARON
      MapItem {0x8Fu, Rune(0x0179u)}, // LATIN CAPITAL LETTER Z WITH ACUTE
      MapItem {0x91u, Rune(0x2018u)}, // LEFT SINGLE QUOTATION MARK
      MapItem {0x92u, Rune(0x2019u)}, // RIGHT SINGLE QUOTATION MARK
      MapItem {0x93u, Rune(0x201Cu)}, // LEFT DOUBLE QUOTATION MARK
      MapItem {0x94u, Rune(0x201Du)}, // RIGHT DOUBLE QUOTATION MARK
      MapItem {0x95u, Rune(0x2022u)}, // BULLET
      MapItem {0x96u, Rune(0x2013u)}, // EN DASH
      MapItem {0x97u, Rune(0x2014u)}, // EM DASH
      MapItem {0x99u, Rune(0x2122u)}, // TRADE MARK SIGN
      MapItem {0x9Au, Rune(0x0161u)}, // LATIN SMALL LETTER S WITH CARON
      MapItem {0x9Bu, Rune(0x203Au)}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
      MapItem {0x9Cu, Rune(0x015Bu)}, // LATIN SMALL LETTER S WITH ACUTE
      MapItem {0x9Du, Rune(0x0165u)}, // LATIN SMALL LETTER T WITH CARON
      MapItem {0x9Eu, Rune(0x017Eu)}, // LATIN SMALL LETTER Z WITH CARON
      MapItem {0x9Fu, Rune(0x017Au)}, // LATIN SMALL LETTER Z WITH ACUTE
      MapItem {0xA0u, Rune(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, Rune(0x02C7u)}, // CARON
      MapItem {0xA2u, Rune(0x02D8u)}, // BREVE
      MapItem {0xA3u, Rune(0x0141u)}, // LATIN CAPITAL LETTER L WITH STROKE
      MapItem {0xA4u, Rune(0x00A4u)}, // CURRENCY SIGN
      MapItem {0xA5u, Rune(0x0104u)}, // LATIN CAPITAL LETTER A WITH OGONEK
      MapItem {0xA6u, Rune(0x00A6u)}, // BROKEN BAR
      MapItem {0xA7u, Rune(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, Rune(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, Rune(0x00A9u)}, // COPYRIGHT SIGN
      MapItem {0xAAu, Rune(0x015Eu)}, // LATIN CAPITAL LETTER S WITH CEDILLA
      MapItem {0xABu, Rune(0x00ABu)}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, Rune(0x00ACu)}, // NOT SIGN
      MapItem {0xADu, Rune(0x00ADu)}, // SOFT HYPHEN
      MapItem {0xAEu, Rune(0x00AEu)}, // REGISTERED SIGN
      MapItem {0xAFu, Rune(0x017Bu)}, // LATIN CAPITAL LETTER Z WITH DOT ABOVE
      MapItem {0xB0u, Rune(0x00B0u)}, // DEGREE SIGN
      MapItem {0xB1u, Rune(0x00B1u)}, // PLUS-MINUS SIGN
      MapItem {0xB2u, Rune(0x02DBu)}, // OGONEK
      MapItem {0xB3u, Rune(0x0142u)}, // LATIN SMALL LETTER L WITH STROKE
      MapItem {0xB4u, Rune(0x00B4u)}, // ACUTE ACCENT
      MapItem {0xB5u, Rune(0x00B5u)}, // MICRO SIGN
      MapItem {0xB6u, Rune(0x00B6u)}, // PILCROW SIGN
      MapItem {0xB7u, Rune(0x00B7u)}, // MIDDLE DOT
      MapItem {0xB8u, Rune(0x00B8u)}, // CEDILLA
      MapItem {0xB9u, Rune(0x0105u)}, // LATIN SMALL LETTER A WITH OGONEK
      MapItem {0xBAu, Rune(0x015Fu)}, // LATIN SMALL LETTER S WITH CEDILLA
      MapItem {0xBBu, Rune(0x00BBu)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, Rune(0x013Du)}, // LATIN CAPITAL LETTER L WITH CARON
      MapItem {0xBDu, Rune(0x02DDu)}, // DOUBLE ACUTE ACCENT
      MapItem {0xBEu, Rune(0x013Eu)}, // LATIN SMALL LETTER L WITH CARON
      MapItem {0xBFu, Rune(0x017Cu)}, // LATIN SMALL LETTER Z WITH DOT ABOVE
      MapItem {0xC0u, Rune(0x0154u)}, // LATIN CAPITAL LETTER R WITH ACUTE
      MapItem {0xC1u, Rune(0x00C1u)}, // LATIN CAPITAL LETTER A WITH ACUTE
      MapItem {0xC2u, Rune(0x00C2u)}, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
      MapItem {0xC3u, Rune(0x0102u)}, // LATIN CAPITAL LETTER A WITH BREVE
      MapItem {0xC4u, Rune(0x00C4u)}, // LATIN CAPITAL LETTER A WITH DIAERESIS
      MapItem {0xC5u, Rune(0x0139u)}, // LATIN CAPITAL LETTER L WITH ACUTE
      MapItem {0xC6u, Rune(0x0106u)}, // LATIN CAPITAL LETTER C WITH ACUTE
      MapItem {0xC7u, Rune(0x00C7u)}, // LATIN CAPITAL LETTER C WITH CEDILLA
      MapItem {0xC8u, Rune(0x010Cu)}, // LATIN CAPITAL LETTER C WITH CARON
      MapItem {0xC9u, Rune(0x00C9u)}, // LATIN CAPITAL LETTER E WITH ACUTE
      MapItem {0xCAu, Rune(0x0118u)}, // LATIN CAPITAL LETTER E WITH OGONEK
      MapItem {0xCBu, Rune(0x00CBu)}, // LATIN CAPITAL LETTER E WITH DIAERESIS
      MapItem {0xCCu, Rune(0x011Au)}, // LATIN CAPITAL LETTER E WITH CARON
      MapItem {0xCDu, Rune(0x00CDu)}, // LATIN CAPITAL LETTER I WITH ACUTE
      MapItem {0xCEu, Rune(0x00CEu)}, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
      MapItem {0xCFu, Rune(0x010Eu)}, // LATIN CAPITAL LETTER D WITH CARON
      MapItem {0xD0u, Rune(0x0110u)}, // LATIN CAPITAL LETTER D WITH STROKE
      MapItem {0xD1u, Rune(0x0143u)}, // LATIN CAPITAL LETTER N WITH ACUTE
      MapItem {0xD2u, Rune(0x0147u)}, // LATIN CAPITAL LETTER N WITH CARON
      MapItem {0xD3u, Rune(0x00D3u)}, // LATIN CAPITAL LETTER O WITH ACUTE
      MapItem {0xD4u, Rune(0x00D4u)}, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
      MapItem {0xD5u, Rune(0x0150u)}, // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
      MapItem {0xD6u, Rune(0x00D6u)}, // LATIN CAPITAL LETTER O WITH DIAERESIS
      MapItem {0xD7u, Rune(0x00D7u)}, // MULTIPLICATION SIGN
      MapItem {0xD8u, Rune(0x0158u)}, // LATIN CAPITAL LETTER R WITH CARON
      MapItem {0xD9u, Rune(0x016Eu)}, // LATIN CAPITAL LETTER U WITH RING ABOVE
      MapItem {0xDAu, Rune(0x00DAu)}, // LATIN CAPITAL LETTER U WITH ACUTE
      MapItem {0xDBu, Rune(0x0170u)}, // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
      MapItem {0xDCu, Rune(0x00DCu)}, // LATIN CAPITAL LETTER U WITH DIAERESIS
      MapItem {0xDDu, Rune(0x00DDu)}, // LATIN CAPITAL LETTER Y WITH ACUTE
      MapItem {0xDEu, Rune(0x0162u)}, // LATIN CAPITAL LETTER T WITH CEDILLA
      MapItem {0xDFu, Rune(0x00DFu)}, // LATIN SMALL LETTER SHARP S
      MapItem {0xE0u, Rune(0x0155u)}, // LATIN SMALL LETTER R WITH ACUTE
      MapItem {0xE1u, Rune(0x00E1u)}, // LATIN SMALL LETTER A WITH ACUTE
      MapItem {0xE2u, Rune(0x00E2u)}, // LATIN SMALL LETTER A WITH CIRCUMFLEX
      MapItem {0xE3u, Rune(0x0103u)}, // LATIN SMALL LETTER A WITH BREVE
      MapItem {0xE4u, Rune(0x00E4u)}, // LATIN SMALL LETTER A WITH DIAERESIS
      MapItem {0xE5u, Rune(0x013Au)}, // LATIN SMALL LETTER L WITH ACUTE
      MapItem {0xE6u, Rune(0x0107u)}, // LATIN SMALL LETTER C WITH ACUTE
      MapItem {0xE7u, Rune(0x00E7u)}, // LATIN SMALL LETTER C WITH CEDILLA
      MapItem {0xE8u, Rune(0x010Du)}, // LATIN SMALL LETTER C WITH CARON
      MapItem {0xE9u, Rune(0x00E9u)}, // LATIN SMALL LETTER E WITH ACUTE
      MapItem {0xEAu, Rune(0x0119u)}, // LATIN SMALL LETTER E WITH OGONEK
      MapItem {0xEBu, Rune(0x00EBu)}, // LATIN SMALL LETTER E WITH DIAERESIS
      MapItem {0xECu, Rune(0x011Bu)}, // LATIN SMALL LETTER E WITH CARON
      MapItem {0xEDu, Rune(0x00EDu)}, // LATIN SMALL LETTER I WITH ACUTE
      MapItem {0xEEu, Rune(0x00EEu)}, // LATIN SMALL LETTER I WITH CIRCUMFLEX
      MapItem {0xEFu, Rune(0x010Fu)}, // LATIN SMALL LETTER D WITH CARON
      MapItem {0xF0u, Rune(0x0111u)}, // LATIN SMALL LETTER D WITH STROKE
      MapItem {0xF1u, Rune(0x0144u)}, // LATIN SMALL LETTER N WITH ACUTE
      MapItem {0xF2u, Rune(0x0148u)}, // LATIN SMALL LETTER N WITH CARON
      MapItem {0xF3u, Rune(0x00F3u)}, // LATIN SMALL LETTER O WITH ACUTE
      MapItem {0xF4u, Rune(0x00F4u)}, // LATIN SMALL LETTER O WITH CIRCUMFLEX
      MapItem {0xF5u, Rune(0x0151u)}, // LATIN SMALL LETTER O WITH DOUBLE ACUTE
      MapItem {0xF6u, Rune(0x00F6u)}, // LATIN SMALL LETTER O WITH DIAERESIS
      MapItem {0xF7u, Rune(0x00F7u)}, // DIVISION SIGN
      MapItem {0xF8u, Rune(0x0159u)}, // LATIN SMALL LETTER R WITH CARON
      MapItem {0xF9u, Rune(0x016Fu)}, // LATIN SMALL LETTER U WITH RING ABOVE
      MapItem {0xFAu, Rune(0x00FAu)}, // LATIN SMALL LETTER U WITH ACUTE
      MapItem {0xFBu, Rune(0x0171u)}, // LATIN SMALL LETTER U WITH DOUBLE ACUTE
      MapItem {0xFCu, Rune(0x00FCu)}, // LATIN SMALL LETTER U WITH DIAERESIS
      MapItem {0xFDu, Rune(0x00FDu)}, // LATIN SMALL LETTER Y WITH ACUTE
      MapItem {0xFEu, Rune(0x0163u)}, // LATIN SMALL LETTER T WITH CEDILLA
      MapItem {0xFFu, Rune(0x02D9u)}, // DOT ABOVE
    };

  public:
    Windows1250Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~Windows1250Encoding() noexcept override = default;
  };
}