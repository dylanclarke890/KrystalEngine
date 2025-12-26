#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys::Text
{
  class ISO_8859_10_Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"ISO-8859-10";
    static constexpr uint32 MIBenum = 13u;
    static constexpr uint32 WindowsCodePage = 28'600u;

    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, UnicodeCodepoint(0x0080u)}, //	<control>
      MapItem {0x81u, UnicodeCodepoint(0x0081u)}, //	<control>
      MapItem {0x82u, UnicodeCodepoint(0x0082u)}, //	<control>
      MapItem {0x83u, UnicodeCodepoint(0x0083u)}, //	<control>
      MapItem {0x84u, UnicodeCodepoint(0x0084u)}, //	<control>
      MapItem {0x85u, UnicodeCodepoint(0x0085u)}, //	<control>
      MapItem {0x86u, UnicodeCodepoint(0x0086u)}, //	<control>
      MapItem {0x87u, UnicodeCodepoint(0x0087u)}, //	<control>
      MapItem {0x88u, UnicodeCodepoint(0x0088u)}, //	<control>
      MapItem {0x89u, UnicodeCodepoint(0x0089u)}, //	<control>
      MapItem {0x8Au, UnicodeCodepoint(0x008Au)}, //	<control>
      MapItem {0x8Bu, UnicodeCodepoint(0x008Bu)}, //	<control>
      MapItem {0x8Cu, UnicodeCodepoint(0x008Cu)}, //	<control>
      MapItem {0x8Du, UnicodeCodepoint(0x008Du)}, //	<control>
      MapItem {0x8Eu, UnicodeCodepoint(0x008Eu)}, //	<control>
      MapItem {0x8Fu, UnicodeCodepoint(0x008Fu)}, //	<control>
      MapItem {0x90u, UnicodeCodepoint(0x0090u)}, //	<control>
      MapItem {0x91u, UnicodeCodepoint(0x0091u)}, //	<control>
      MapItem {0x92u, UnicodeCodepoint(0x0092u)}, //	<control>
      MapItem {0x93u, UnicodeCodepoint(0x0093u)}, //	<control>
      MapItem {0x94u, UnicodeCodepoint(0x0094u)}, //	<control>
      MapItem {0x95u, UnicodeCodepoint(0x0095u)}, //	<control>
      MapItem {0x96u, UnicodeCodepoint(0x0096u)}, //	<control>
      MapItem {0x97u, UnicodeCodepoint(0x0097u)}, //	<control>
      MapItem {0x98u, UnicodeCodepoint(0x0098u)}, //	<control>
      MapItem {0x99u, UnicodeCodepoint(0x0099u)}, //	<control>
      MapItem {0x9Au, UnicodeCodepoint(0x009Au)}, //	<control>
      MapItem {0x9Bu, UnicodeCodepoint(0x009Bu)}, //	<control>
      MapItem {0x9Cu, UnicodeCodepoint(0x009Cu)}, //	<control>
      MapItem {0x9Du, UnicodeCodepoint(0x009Du)}, //	<control>
      MapItem {0x9Eu, UnicodeCodepoint(0x009Eu)}, //	<control>
      MapItem {0x9Fu, UnicodeCodepoint(0x009Fu)}, //	<control>
      MapItem {0xA0u, UnicodeCodepoint(0x00A0u)}, //	NO-BREAK SPACE
      MapItem {0xA1u, UnicodeCodepoint(0x0104u)}, //	LATIN CAPITAL LETTER A WITH OGONEK
      MapItem {0xA2u, UnicodeCodepoint(0x0112u)}, //	LATIN CAPITAL LETTER E WITH MACRON
      MapItem {0xA3u, UnicodeCodepoint(0x0122u)}, //	LATIN CAPITAL LETTER G WITH CEDILLA
      MapItem {0xA4u, UnicodeCodepoint(0x012Au)}, //	LATIN CAPITAL LETTER I WITH MACRON
      MapItem {0xA5u, UnicodeCodepoint(0x0128u)}, //	LATIN CAPITAL LETTER I WITH TILDE
      MapItem {0xA6u, UnicodeCodepoint(0x0136u)}, //	LATIN CAPITAL LETTER K WITH CEDILLA
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, //	SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x013Bu)}, //	LATIN CAPITAL LETTER L WITH CEDILLA
      MapItem {0xA9u, UnicodeCodepoint(0x0110u)}, //	LATIN CAPITAL LETTER D WITH STROKE
      MapItem {0xAAu, UnicodeCodepoint(0x0160u)}, //	LATIN CAPITAL LETTER S WITH CARON
      MapItem {0xABu, UnicodeCodepoint(0x0166u)}, //	LATIN CAPITAL LETTER T WITH STROKE
      MapItem {0xACu, UnicodeCodepoint(0x017Du)}, //	LATIN CAPITAL LETTER Z WITH CARON
      MapItem {0xADu, UnicodeCodepoint(0x00ADu)}, //	SOFT HYPHEN
      MapItem {0xAEu, UnicodeCodepoint(0x016Au)}, //	LATIN CAPITAL LETTER U WITH MACRON
      MapItem {0xAFu, UnicodeCodepoint(0x014Au)}, //	LATIN CAPITAL LETTER ENG
      MapItem {0xB0u, UnicodeCodepoint(0x00B0u)}, //	DEGREE SIGN
      MapItem {0xB1u, UnicodeCodepoint(0x0105u)}, //	LATIN SMALL LETTER A WITH OGONEK
      MapItem {0xB2u, UnicodeCodepoint(0x0113u)}, //	LATIN SMALL LETTER E WITH MACRON
      MapItem {0xB3u, UnicodeCodepoint(0x0123u)}, //	LATIN SMALL LETTER G WITH CEDILLA
      MapItem {0xB4u, UnicodeCodepoint(0x012Bu)}, //	LATIN SMALL LETTER I WITH MACRON
      MapItem {0xB5u, UnicodeCodepoint(0x0129u)}, //	LATIN SMALL LETTER I WITH TILDE
      MapItem {0xB6u, UnicodeCodepoint(0x0137u)}, //	LATIN SMALL LETTER K WITH CEDILLA
      MapItem {0xB7u, UnicodeCodepoint(0x00B7u)}, //	MIDDLE DOT
      MapItem {0xB8u, UnicodeCodepoint(0x013Cu)}, //	LATIN SMALL LETTER L WITH CEDILLA
      MapItem {0xB9u, UnicodeCodepoint(0x0111u)}, //	LATIN SMALL LETTER D WITH STROKE
      MapItem {0xBAu, UnicodeCodepoint(0x0161u)}, //	LATIN SMALL LETTER S WITH CARON
      MapItem {0xBBu, UnicodeCodepoint(0x0167u)}, //	LATIN SMALL LETTER T WITH STROKE
      MapItem {0xBCu, UnicodeCodepoint(0x017Eu)}, //	LATIN SMALL LETTER Z WITH CARON
      MapItem {0xBDu, UnicodeCodepoint(0x2015u)}, //	HORIZONTAL BAR
      MapItem {0xBEu, UnicodeCodepoint(0x016Bu)}, //	LATIN SMALL LETTER U WITH MACRON
      MapItem {0xBFu, UnicodeCodepoint(0x014Bu)}, //	LATIN SMALL LETTER ENG
      MapItem {0xC0u, UnicodeCodepoint(0x0100u)}, //	LATIN CAPITAL LETTER A WITH MACRON
      MapItem {0xC1u, UnicodeCodepoint(0x00C1u)}, //	LATIN CAPITAL LETTER A WITH ACUTE
      MapItem {0xC2u, UnicodeCodepoint(0x00C2u)}, //	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
      MapItem {0xC3u, UnicodeCodepoint(0x00C3u)}, //	LATIN CAPITAL LETTER A WITH TILDE
      MapItem {0xC4u, UnicodeCodepoint(0x00C4u)}, //	LATIN CAPITAL LETTER A WITH DIAERESIS
      MapItem {0xC5u, UnicodeCodepoint(0x00C5u)}, //	LATIN CAPITAL LETTER A WITH RING ABOVE
      MapItem {0xC6u, UnicodeCodepoint(0x00C6u)}, //	LATIN CAPITAL LETTER AE
      MapItem {0xC7u, UnicodeCodepoint(0x012Eu)}, //	LATIN CAPITAL LETTER I WITH OGONEK
      MapItem {0xC8u, UnicodeCodepoint(0x010Cu)}, //	LATIN CAPITAL LETTER C WITH CARON
      MapItem {0xC9u, UnicodeCodepoint(0x00C9u)}, //	LATIN CAPITAL LETTER E WITH ACUTE
      MapItem {0xCAu, UnicodeCodepoint(0x0118u)}, //	LATIN CAPITAL LETTER E WITH OGONEK
      MapItem {0xCBu, UnicodeCodepoint(0x00CBu)}, //	LATIN CAPITAL LETTER E WITH DIAERESIS
      MapItem {0xCCu, UnicodeCodepoint(0x0116u)}, //	LATIN CAPITAL LETTER E WITH DOT ABOVE
      MapItem {0xCDu, UnicodeCodepoint(0x00CDu)}, //	LATIN CAPITAL LETTER I WITH ACUTE
      MapItem {0xCEu, UnicodeCodepoint(0x00CEu)}, //	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
      MapItem {0xCFu, UnicodeCodepoint(0x00CFu)}, //	LATIN CAPITAL LETTER I WITH DIAERESIS
      MapItem {0xD0u, UnicodeCodepoint(0x00D0u)}, //	LATIN CAPITAL LETTER ETH (Icelandic)
      MapItem {0xD1u, UnicodeCodepoint(0x0145u)}, //	LATIN CAPITAL LETTER N WITH CEDILLA
      MapItem {0xD2u, UnicodeCodepoint(0x014Cu)}, //	LATIN CAPITAL LETTER O WITH MACRON
      MapItem {0xD3u, UnicodeCodepoint(0x00D3u)}, //	LATIN CAPITAL LETTER O WITH ACUTE
      MapItem {0xD4u, UnicodeCodepoint(0x00D4u)}, //	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
      MapItem {0xD5u, UnicodeCodepoint(0x00D5u)}, //	LATIN CAPITAL LETTER O WITH TILDE
      MapItem {0xD6u, UnicodeCodepoint(0x00D6u)}, //	LATIN CAPITAL LETTER O WITH DIAERESIS
      MapItem {0xD7u, UnicodeCodepoint(0x0168u)}, //	LATIN CAPITAL LETTER U WITH TILDE
      MapItem {0xD8u, UnicodeCodepoint(0x00D8u)}, //	LATIN CAPITAL LETTER O WITH STROKE
      MapItem {0xD9u, UnicodeCodepoint(0x0172u)}, //	LATIN CAPITAL LETTER U WITH OGONEK
      MapItem {0xDAu, UnicodeCodepoint(0x00DAu)}, //	LATIN CAPITAL LETTER U WITH ACUTE
      MapItem {0xDBu, UnicodeCodepoint(0x00DBu)}, //	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
      MapItem {0xDCu, UnicodeCodepoint(0x00DCu)}, //	LATIN CAPITAL LETTER U WITH DIAERESIS
      MapItem {0xDDu, UnicodeCodepoint(0x00DDu)}, //	LATIN CAPITAL LETTER Y WITH ACUTE
      MapItem {0xDEu, UnicodeCodepoint(0x00DEu)}, //	LATIN CAPITAL LETTER THORN (Icelandic)
      MapItem {0xDFu, UnicodeCodepoint(0x00DFu)}, //	LATIN SMALL LETTER SHARP S (German)
      MapItem {0xE0u, UnicodeCodepoint(0x0101u)}, //	LATIN SMALL LETTER A WITH MACRON
      MapItem {0xE1u, UnicodeCodepoint(0x00E1u)}, //	LATIN SMALL LETTER A WITH ACUTE
      MapItem {0xE2u, UnicodeCodepoint(0x00E2u)}, //	LATIN SMALL LETTER A WITH CIRCUMFLEX
      MapItem {0xE3u, UnicodeCodepoint(0x00E3u)}, //	LATIN SMALL LETTER A WITH TILDE
      MapItem {0xE4u, UnicodeCodepoint(0x00E4u)}, //	LATIN SMALL LETTER A WITH DIAERESIS
      MapItem {0xE5u, UnicodeCodepoint(0x00E5u)}, //	LATIN SMALL LETTER A WITH RING ABOVE
      MapItem {0xE6u, UnicodeCodepoint(0x00E6u)}, //	LATIN SMALL LETTER AE
      MapItem {0xE7u, UnicodeCodepoint(0x012Fu)}, //	LATIN SMALL LETTER I WITH OGONEK
      MapItem {0xE8u, UnicodeCodepoint(0x010Du)}, //	LATIN SMALL LETTER C WITH CARON
      MapItem {0xE9u, UnicodeCodepoint(0x00E9u)}, //	LATIN SMALL LETTER E WITH ACUTE
      MapItem {0xEAu, UnicodeCodepoint(0x0119u)}, //	LATIN SMALL LETTER E WITH OGONEK
      MapItem {0xEBu, UnicodeCodepoint(0x00EBu)}, //	LATIN SMALL LETTER E WITH DIAERESIS
      MapItem {0xECu, UnicodeCodepoint(0x0117u)}, //	LATIN SMALL LETTER E WITH DOT ABOVE
      MapItem {0xEDu, UnicodeCodepoint(0x00EDu)}, //	LATIN SMALL LETTER I WITH ACUTE
      MapItem {0xEEu, UnicodeCodepoint(0x00EEu)}, //	LATIN SMALL LETTER I WITH CIRCUMFLEX
      MapItem {0xEFu, UnicodeCodepoint(0x00EFu)}, //	LATIN SMALL LETTER I WITH DIAERESIS
      MapItem {0xF0u, UnicodeCodepoint(0x00F0u)}, //	LATIN SMALL LETTER ETH (Icelandic)
      MapItem {0xF1u, UnicodeCodepoint(0x0146u)}, //	LATIN SMALL LETTER N WITH CEDILLA
      MapItem {0xF2u, UnicodeCodepoint(0x014Du)}, //	LATIN SMALL LETTER O WITH MACRON
      MapItem {0xF3u, UnicodeCodepoint(0x00F3u)}, //	LATIN SMALL LETTER O WITH ACUTE
      MapItem {0xF4u, UnicodeCodepoint(0x00F4u)}, //	LATIN SMALL LETTER O WITH CIRCUMFLEX
      MapItem {0xF5u, UnicodeCodepoint(0x00F5u)}, //	LATIN SMALL LETTER O WITH TILDE
      MapItem {0xF6u, UnicodeCodepoint(0x00F6u)}, //	LATIN SMALL LETTER O WITH DIAERESIS
      MapItem {0xF7u, UnicodeCodepoint(0x0169u)}, //	LATIN SMALL LETTER U WITH TILDE
      MapItem {0xF8u, UnicodeCodepoint(0x00F8u)}, //	LATIN SMALL LETTER O WITH STROKE
      MapItem {0xF9u, UnicodeCodepoint(0x0173u)}, //	LATIN SMALL LETTER U WITH OGONEK
      MapItem {0xFAu, UnicodeCodepoint(0x00FAu)}, //	LATIN SMALL LETTER U WITH ACUTE
      MapItem {0xFBu, UnicodeCodepoint(0x00FBu)}, //	LATIN SMALL LETTER U WITH CIRCUMFLEX
      MapItem {0xFCu, UnicodeCodepoint(0x00FCu)}, //	LATIN SMALL LETTER U WITH DIAERESIS
      MapItem {0xFDu, UnicodeCodepoint(0x00FDu)}, //	LATIN SMALL LETTER Y WITH ACUTE
      MapItem {0xFEu, UnicodeCodepoint(0x00FEu)}, //	LATIN SMALL LETTER THORN (Icelandic)
      MapItem {0xFFu, UnicodeCodepoint(0x0138u)}, //	LATIN SMALL LETTER KRA
    };

  public:
    ISO_8859_10_Encoding() noexcept
        : SingleByteEncoding({Name, MIBenum, WindowsCodePage}, LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_10_Encoding() noexcept override = default;
  };
}