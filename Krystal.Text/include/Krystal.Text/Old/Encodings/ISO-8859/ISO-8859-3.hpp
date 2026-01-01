#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class ISO_8859_3_Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"ISO-8859-3",
        .Aliases = {u8"csisolatin3", u8"iso-ir-109", u8"iso8859-3", u8"iso88593", u8"iso_8859-3",
                    u8"iso_8859-3:1988", u8"l3", u8"latin3", u8"iso-8859-3"},
        .MIBenum = MIBenum {6u},
        .Win32CodePage = Win32CodePage {28'593u},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, Rune(0x0080u)}, // <control>
      MapItem {0x81u, Rune(0x0081u)}, // <control>
      MapItem {0x82u, Rune(0x0082u)}, // <control>
      MapItem {0x83u, Rune(0x0083u)}, // <control>
      MapItem {0x84u, Rune(0x0084u)}, // <control>
      MapItem {0x85u, Rune(0x0085u)}, // <control>
      MapItem {0x86u, Rune(0x0086u)}, // <control>
      MapItem {0x87u, Rune(0x0087u)}, // <control>
      MapItem {0x88u, Rune(0x0088u)}, // <control>
      MapItem {0x89u, Rune(0x0089u)}, // <control>
      MapItem {0x8Au, Rune(0x008Au)}, // <control>
      MapItem {0x8Bu, Rune(0x008Bu)}, // <control>
      MapItem {0x8Cu, Rune(0x008Cu)}, // <control>
      MapItem {0x8Du, Rune(0x008Du)}, // <control>
      MapItem {0x8Eu, Rune(0x008Eu)}, // <control>
      MapItem {0x8Fu, Rune(0x008Fu)}, // <control>
      MapItem {0x90u, Rune(0x0090u)}, // <control>
      MapItem {0x91u, Rune(0x0091u)}, // <control>
      MapItem {0x92u, Rune(0x0092u)}, // <control>
      MapItem {0x93u, Rune(0x0093u)}, // <control>
      MapItem {0x94u, Rune(0x0094u)}, // <control>
      MapItem {0x95u, Rune(0x0095u)}, // <control>
      MapItem {0x96u, Rune(0x0096u)}, // <control>
      MapItem {0x97u, Rune(0x0097u)}, // <control>
      MapItem {0x98u, Rune(0x0098u)}, // <control>
      MapItem {0x99u, Rune(0x0099u)}, // <control>
      MapItem {0x9Au, Rune(0x009Au)}, // <control>
      MapItem {0x9Bu, Rune(0x009Bu)}, // <control>
      MapItem {0x9Cu, Rune(0x009Cu)}, // <control>
      MapItem {0x9Du, Rune(0x009Du)}, // <control>
      MapItem {0x9Eu, Rune(0x009Eu)}, // <control>
      MapItem {0x9Fu, Rune(0x009Fu)}, // <control>
      MapItem {0xA0u, Rune(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, Rune(0x0126u)}, // LATIN CAPITAL LETTER H WITH STROKE
      MapItem {0xA2u, Rune(0x02D8u)}, // BREVE
      MapItem {0xA3u, Rune(0x00A3u)}, // POUND SIGN
      MapItem {0xA4u, Rune(0x00A4u)}, // CURRENCY SIGN
      MapItem {0xA6u, Rune(0x0124u)}, // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
      MapItem {0xA7u, Rune(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, Rune(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, Rune(0x0130u)}, // LATIN CAPITAL LETTER I WITH DOT ABOVE
      MapItem {0xAAu, Rune(0x015Eu)}, // LATIN CAPITAL LETTER S WITH CEDILLA
      MapItem {0xABu, Rune(0x011Eu)}, // LATIN CAPITAL LETTER G WITH BREVE
      MapItem {0xACu, Rune(0x0134u)}, // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
      MapItem {0xADu, Rune(0x00ADu)}, // SOFT HYPHEN
      MapItem {0xAFu, Rune(0x017Bu)}, // LATIN CAPITAL LETTER Z WITH DOT ABOVE
      MapItem {0xB0u, Rune(0x00B0u)}, // DEGREE SIGN
      MapItem {0xB1u, Rune(0x0127u)}, // LATIN SMALL LETTER H WITH STROKE
      MapItem {0xB2u, Rune(0x00B2u)}, // SUPERSCRIPT TWO
      MapItem {0xB3u, Rune(0x00B3u)}, // SUPERSCRIPT THREE
      MapItem {0xB4u, Rune(0x00B4u)}, // ACUTE ACCENT
      MapItem {0xB5u, Rune(0x00B5u)}, // MICRO SIGN
      MapItem {0xB6u, Rune(0x0125u)}, // LATIN SMALL LETTER H WITH CIRCUMFLEX
      MapItem {0xB7u, Rune(0x00B7u)}, // MIDDLE DOT
      MapItem {0xB8u, Rune(0x00B8u)}, // CEDILLA
      MapItem {0xB9u, Rune(0x0131u)}, // LATIN SMALL LETTER DOTLESS I
      MapItem {0xBAu, Rune(0x015Fu)}, // LATIN SMALL LETTER S WITH CEDILLA
      MapItem {0xBBu, Rune(0x011Fu)}, // LATIN SMALL LETTER G WITH BREVE
      MapItem {0xBCu, Rune(0x0135u)}, // LATIN SMALL LETTER J WITH CIRCUMFLEX
      MapItem {0xBDu, Rune(0x00BDu)}, // VULGAR FRACTION ONE HALF
      MapItem {0xBFu, Rune(0x017Cu)}, // LATIN SMALL LETTER Z WITH DOT ABOVE
      MapItem {0xC0u, Rune(0x00C0u)}, // LATIN CAPITAL LETTER A WITH GRAVE
      MapItem {0xC1u, Rune(0x00C1u)}, // LATIN CAPITAL LETTER A WITH ACUTE
      MapItem {0xC2u, Rune(0x00C2u)}, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
      MapItem {0xC4u, Rune(0x00C4u)}, // LATIN CAPITAL LETTER A WITH DIAERESIS
      MapItem {0xC5u, Rune(0x010Au)}, // LATIN CAPITAL LETTER C WITH DOT ABOVE
      MapItem {0xC6u, Rune(0x0108u)}, // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
      MapItem {0xC7u, Rune(0x00C7u)}, // LATIN CAPITAL LETTER C WITH CEDILLA
      MapItem {0xC8u, Rune(0x00C8u)}, // LATIN CAPITAL LETTER E WITH GRAVE
      MapItem {0xC9u, Rune(0x00C9u)}, // LATIN CAPITAL LETTER E WITH ACUTE
      MapItem {0xCAu, Rune(0x00CAu)}, // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
      MapItem {0xCBu, Rune(0x00CBu)}, // LATIN CAPITAL LETTER E WITH DIAERESIS
      MapItem {0xCCu, Rune(0x00CCu)}, // LATIN CAPITAL LETTER I WITH GRAVE
      MapItem {0xCDu, Rune(0x00CDu)}, // LATIN CAPITAL LETTER I WITH ACUTE
      MapItem {0xCEu, Rune(0x00CEu)}, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
      MapItem {0xCFu, Rune(0x00CFu)}, // LATIN CAPITAL LETTER I WITH DIAERESIS
      MapItem {0xD1u, Rune(0x00D1u)}, // LATIN CAPITAL LETTER N WITH TILDE
      MapItem {0xD2u, Rune(0x00D2u)}, // LATIN CAPITAL LETTER O WITH GRAVE
      MapItem {0xD3u, Rune(0x00D3u)}, // LATIN CAPITAL LETTER O WITH ACUTE
      MapItem {0xD4u, Rune(0x00D4u)}, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
      MapItem {0xD5u, Rune(0x0120u)}, // LATIN CAPITAL LETTER G WITH DOT ABOVE
      MapItem {0xD6u, Rune(0x00D6u)}, // LATIN CAPITAL LETTER O WITH DIAERESIS
      MapItem {0xD7u, Rune(0x00D7u)}, // MULTIPLICATION SIGN
      MapItem {0xD8u, Rune(0x011Cu)}, // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
      MapItem {0xD9u, Rune(0x00D9u)}, // LATIN CAPITAL LETTER U WITH GRAVE
      MapItem {0xDAu, Rune(0x00DAu)}, // LATIN CAPITAL LETTER U WITH ACUTE
      MapItem {0xDBu, Rune(0x00DBu)}, // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
      MapItem {0xDCu, Rune(0x00DCu)}, // LATIN CAPITAL LETTER U WITH DIAERESIS
      MapItem {0xDDu, Rune(0x016Cu)}, // LATIN CAPITAL LETTER U WITH BREVE
      MapItem {0xDEu, Rune(0x015Cu)}, // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
      MapItem {0xDFu, Rune(0x00DFu)}, // LATIN SMALL LETTER SHARP S
      MapItem {0xE0u, Rune(0x00E0u)}, // LATIN SMALL LETTER A WITH GRAVE
      MapItem {0xE1u, Rune(0x00E1u)}, // LATIN SMALL LETTER A WITH ACUTE
      MapItem {0xE2u, Rune(0x00E2u)}, // LATIN SMALL LETTER A WITH CIRCUMFLEX
      MapItem {0xE4u, Rune(0x00E4u)}, // LATIN SMALL LETTER A WITH DIAERESIS
      MapItem {0xE5u, Rune(0x010Bu)}, // LATIN SMALL LETTER C WITH DOT ABOVE
      MapItem {0xE6u, Rune(0x0109u)}, // LATIN SMALL LETTER C WITH CIRCUMFLEX
      MapItem {0xE7u, Rune(0x00E7u)}, // LATIN SMALL LETTER C WITH CEDILLA
      MapItem {0xE8u, Rune(0x00E8u)}, // LATIN SMALL LETTER E WITH GRAVE
      MapItem {0xE9u, Rune(0x00E9u)}, // LATIN SMALL LETTER E WITH ACUTE
      MapItem {0xEAu, Rune(0x00EAu)}, // LATIN SMALL LETTER E WITH CIRCUMFLEX
      MapItem {0xEBu, Rune(0x00EBu)}, // LATIN SMALL LETTER E WITH DIAERESIS
      MapItem {0xECu, Rune(0x00ECu)}, // LATIN SMALL LETTER I WITH GRAVE
      MapItem {0xEDu, Rune(0x00EDu)}, // LATIN SMALL LETTER I WITH ACUTE
      MapItem {0xEEu, Rune(0x00EEu)}, // LATIN SMALL LETTER I WITH CIRCUMFLEX
      MapItem {0xEFu, Rune(0x00EFu)}, // LATIN SMALL LETTER I WITH DIAERESIS
      MapItem {0xF1u, Rune(0x00F1u)}, // LATIN SMALL LETTER N WITH TILDE
      MapItem {0xF2u, Rune(0x00F2u)}, // LATIN SMALL LETTER O WITH GRAVE
      MapItem {0xF3u, Rune(0x00F3u)}, // LATIN SMALL LETTER O WITH ACUTE
      MapItem {0xF4u, Rune(0x00F4u)}, // LATIN SMALL LETTER O WITH CIRCUMFLEX
      MapItem {0xF5u, Rune(0x0121u)}, // LATIN SMALL LETTER G WITH DOT ABOVE
      MapItem {0xF6u, Rune(0x00F6u)}, // LATIN SMALL LETTER O WITH DIAERESIS
      MapItem {0xF7u, Rune(0x00F7u)}, // DIVISION SIGN
      MapItem {0xF8u, Rune(0x011Du)}, // LATIN SMALL LETTER G WITH CIRCUMFLEX
      MapItem {0xF9u, Rune(0x00F9u)}, // LATIN SMALL LETTER U WITH GRAVE
      MapItem {0xFAu, Rune(0x00FAu)}, // LATIN SMALL LETTER U WITH ACUTE
      MapItem {0xFBu, Rune(0x00FBu)}, // LATIN SMALL LETTER U WITH CIRCUMFLEX
      MapItem {0xFCu, Rune(0x00FCu)}, // LATIN SMALL LETTER U WITH DIAERESIS
      MapItem {0xFDu, Rune(0x016Du)}, // LATIN SMALL LETTER U WITH BREVE
      MapItem {0xFEu, Rune(0x015Du)}, // LATIN SMALL LETTER S WITH CIRCUMFLEX
      MapItem {0xFFu, Rune(0x02D9u)}, // DOT ABOVE
    };

  public:
    ISO_8859_3_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_3_Encoding() noexcept override = default;
  };
}