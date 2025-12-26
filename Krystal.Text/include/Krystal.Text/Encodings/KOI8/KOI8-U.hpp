#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys::Text
{
  class KOI8_U_Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview IANA = u8"KOI8-U";
    static constexpr EncodingInfo EncodingInformation = EncodingInfo {IANA, 21'866u};

    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, UnicodeCodepoint(0x2500u)}, //	BOX DRAWINGS LIGHT HORIZONTAL
      MapItem {0x81u, UnicodeCodepoint(0x2502u)}, //	BOX DRAWINGS LIGHT VERTICAL
      MapItem {0x82u, UnicodeCodepoint(0x250Cu)}, //	BOX DRAWINGS LIGHT DOWN AND RIGHT
      MapItem {0x83u, UnicodeCodepoint(0x2510u)}, //	BOX DRAWINGS LIGHT DOWN AND LEFT
      MapItem {0x84u, UnicodeCodepoint(0x2514u)}, //	BOX DRAWINGS LIGHT UP AND RIGHT
      MapItem {0x85u, UnicodeCodepoint(0x2518u)}, //	BOX DRAWINGS LIGHT UP AND LEFT
      MapItem {0x86u, UnicodeCodepoint(0x251Cu)}, //	BOX DRAWINGS LIGHT VERTICAL AND RIGHT
      MapItem {0x87u, UnicodeCodepoint(0x2524u)}, //	BOX DRAWINGS LIGHT VERTICAL AND LEFT
      MapItem {0x88u, UnicodeCodepoint(0x252Cu)}, //	BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
      MapItem {0x89u, UnicodeCodepoint(0x2534u)}, //	BOX DRAWINGS LIGHT UP AND HORIZONTAL
      MapItem {0x8Au, UnicodeCodepoint(0x253Cu)}, //	BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
      MapItem {0x8Bu, UnicodeCodepoint(0x2580u)}, //	UPPER HALF BLOCK
      MapItem {0x8Cu, UnicodeCodepoint(0x2584u)}, //	LOWER HALF BLOCK
      MapItem {0x8Du, UnicodeCodepoint(0x2588u)}, //	FULL BLOCK
      MapItem {0x8Eu, UnicodeCodepoint(0x258Cu)}, //	LEFT HALF BLOCK
      MapItem {0x8Fu, UnicodeCodepoint(0x2590u)}, //	RIGHT HALF BLOCK
      MapItem {0x90u, UnicodeCodepoint(0x2591u)}, //	LIGHT SHADE
      MapItem {0x91u, UnicodeCodepoint(0x2592u)}, //	MEDIUM SHADE
      MapItem {0x92u, UnicodeCodepoint(0x2593u)}, //	DARK SHADE
      MapItem {0x93u, UnicodeCodepoint(0x2320u)}, //	TOP HALF INTEGRAL
      MapItem {0x94u, UnicodeCodepoint(0x25A0u)}, //	BLACK SQUARE
      MapItem {0x95u, UnicodeCodepoint(0x2219u)}, //	BULLET OPERATOR
      MapItem {0x96u, UnicodeCodepoint(0x221Au)}, //	SQUARE ROOT
      MapItem {0x97u, UnicodeCodepoint(0x2248u)}, //	ALMOST EQUAL TO
      MapItem {0x98u, UnicodeCodepoint(0x2264u)}, //	LESS-THAN OR EQUAL TO
      MapItem {0x99u, UnicodeCodepoint(0x2265u)}, //	GREATER-THAN OR EQUAL TO
      MapItem {0x9Au, UnicodeCodepoint(0x00A0u)}, //	NO-BREAK SPACE
      MapItem {0x9Bu, UnicodeCodepoint(0x2321u)}, //	BOTTOM HALF INTEGRAL
      MapItem {0x9Cu, UnicodeCodepoint(0x00B0u)}, //	DEGREE SIGN
      MapItem {0x9Du, UnicodeCodepoint(0x00B2u)}, //	SUPERSCRIPT TWO
      MapItem {0x9Eu, UnicodeCodepoint(0x00B7u)}, //	MIDDLE DOT
      MapItem {0x9Fu, UnicodeCodepoint(0x00F7u)}, //	DIVISION SIGN
      MapItem {0xA0u, UnicodeCodepoint(0x2550u)}, //	BOX DRAWINGS DOUBLE HORIZONTAL
      MapItem {0xA1u, UnicodeCodepoint(0x2551u)}, //	BOX DRAWINGS DOUBLE VERTICAL
      MapItem {0xA2u, UnicodeCodepoint(0x2552u)}, //	BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
      MapItem {0xA3u, UnicodeCodepoint(0x0451u)}, //	CYRILLIC SMALL LETTER IO
      MapItem {0xA4u, UnicodeCodepoint(0x0454u)}, //	CYRILLIC SMALL LETTER UKRAINIAN IE
      MapItem {0xA5u, UnicodeCodepoint(0x2554u)}, //	BOX DRAWINGS DOUBLE DOWN AND RIGHT
      MapItem {0xA6u, UnicodeCodepoint(0x0456u)}, //	CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
      MapItem {0xA7u, UnicodeCodepoint(0x0457u)}, //	CYRILLIC SMALL LETTER YI (UKRAINIAN)
      MapItem {0xA8u, UnicodeCodepoint(0x2557u)}, //	BOX DRAWINGS DOUBLE DOWN AND LEFT
      MapItem {0xA9u, UnicodeCodepoint(0x2558u)}, //	BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
      MapItem {0xAAu, UnicodeCodepoint(0x2559u)}, //	BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
      MapItem {0xABu, UnicodeCodepoint(0x255Au)}, //	BOX DRAWINGS DOUBLE UP AND RIGHT
      MapItem {0xACu, UnicodeCodepoint(0x255Bu)}, //	BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
      MapItem {0xADu, UnicodeCodepoint(0x0491u)}, //	CYRILLIC SMALL LETTER GHE WITH UPTURN
      MapItem {0xAEu, UnicodeCodepoint(0x255Du)}, //	BOX DRAWINGS DOUBLE UP AND LEFT
      MapItem {0xAFu, UnicodeCodepoint(0x255Eu)}, //	BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
      MapItem {0xB0u, UnicodeCodepoint(0x255Fu)}, //	BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
      MapItem {0xB1u, UnicodeCodepoint(0x2560u)}, //	BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
      MapItem {0xB2u, UnicodeCodepoint(0x2561u)}, //	BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
      MapItem {0xB3u, UnicodeCodepoint(0x0401u)}, //	CYRILLIC CAPITAL LETTER IO
      MapItem {0xB4u, UnicodeCodepoint(0x0404u)}, //	CYRILLIC CAPITAL LETTER UKRAINIAN IE
      MapItem {0xB5u, UnicodeCodepoint(0x2563u)}, //	BOX DRAWINGS DOUBLE VERTICAL AND LEFT
      MapItem {0xB6u, UnicodeCodepoint(0x0406u)}, //	CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
      MapItem {0xB7u, UnicodeCodepoint(0x0407u)}, //	CYRILLIC CAPITAL LETTER YI (UKRAINIAN)
      MapItem {0xB8u, UnicodeCodepoint(0x2566u)}, //	BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
      MapItem {0xB9u, UnicodeCodepoint(0x2567u)}, //	BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
      MapItem {0xBAu, UnicodeCodepoint(0x2568u)}, //	BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
      MapItem {0xBBu, UnicodeCodepoint(0x2569u)}, //	BOX DRAWINGS DOUBLE UP AND HORIZONTAL
      MapItem {0xBCu, UnicodeCodepoint(0x256Au)}, //	BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
      MapItem {0xBDu, UnicodeCodepoint(0x0490u)}, //	CYRILLIC CAPITAL LETTER GHE WITH UPTURN
      MapItem {0xBEu, UnicodeCodepoint(0x256Cu)}, //	BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
      MapItem {0xBFu, UnicodeCodepoint(0x00A9u)}, //	COPYRIGHT SIGN
      MapItem {0xC0u, UnicodeCodepoint(0x044Eu)}, //	CYRILLIC SMALL LETTER YU
      MapItem {0xC1u, UnicodeCodepoint(0x0430u)}, //	CYRILLIC SMALL LETTER A
      MapItem {0xC2u, UnicodeCodepoint(0x0431u)}, //	CYRILLIC SMALL LETTER BE
      MapItem {0xC3u, UnicodeCodepoint(0x0446u)}, //	CYRILLIC SMALL LETTER TSE
      MapItem {0xC4u, UnicodeCodepoint(0x0434u)}, //	CYRILLIC SMALL LETTER DE
      MapItem {0xC5u, UnicodeCodepoint(0x0435u)}, //	CYRILLIC SMALL LETTER IE
      MapItem {0xC6u, UnicodeCodepoint(0x0444u)}, //	CYRILLIC SMALL LETTER EF
      MapItem {0xC7u, UnicodeCodepoint(0x0433u)}, //	CYRILLIC SMALL LETTER GHE
      MapItem {0xC8u, UnicodeCodepoint(0x0445u)}, //	CYRILLIC SMALL LETTER HA
      MapItem {0xC9u, UnicodeCodepoint(0x0438u)}, //	CYRILLIC SMALL LETTER I
      MapItem {0xCAu, UnicodeCodepoint(0x0439u)}, //	CYRILLIC SMALL LETTER SHORT I
      MapItem {0xCBu, UnicodeCodepoint(0x043Au)}, //	CYRILLIC SMALL LETTER KA
      MapItem {0xCCu, UnicodeCodepoint(0x043Bu)}, //	CYRILLIC SMALL LETTER EL
      MapItem {0xCDu, UnicodeCodepoint(0x043Cu)}, //	CYRILLIC SMALL LETTER EM
      MapItem {0xCEu, UnicodeCodepoint(0x043Du)}, //	CYRILLIC SMALL LETTER EN
      MapItem {0xCFu, UnicodeCodepoint(0x043Eu)}, //	CYRILLIC SMALL LETTER O
      MapItem {0xD0u, UnicodeCodepoint(0x043Fu)}, //	CYRILLIC SMALL LETTER PE
      MapItem {0xD1u, UnicodeCodepoint(0x044Fu)}, //	CYRILLIC SMALL LETTER YA
      MapItem {0xD2u, UnicodeCodepoint(0x0440u)}, //	CYRILLIC SMALL LETTER ER
      MapItem {0xD3u, UnicodeCodepoint(0x0441u)}, //	CYRILLIC SMALL LETTER ES
      MapItem {0xD4u, UnicodeCodepoint(0x0442u)}, //	CYRILLIC SMALL LETTER TE
      MapItem {0xD5u, UnicodeCodepoint(0x0443u)}, //	CYRILLIC SMALL LETTER U
      MapItem {0xD6u, UnicodeCodepoint(0x0436u)}, //	CYRILLIC SMALL LETTER ZHE
      MapItem {0xD7u, UnicodeCodepoint(0x0432u)}, //	CYRILLIC SMALL LETTER VE
      MapItem {0xD8u, UnicodeCodepoint(0x044Cu)}, //	CYRILLIC SMALL LETTER SOFT SIGN
      MapItem {0xD9u, UnicodeCodepoint(0x044Bu)}, //	CYRILLIC SMALL LETTER YERU
      MapItem {0xDAu, UnicodeCodepoint(0x0437u)}, //	CYRILLIC SMALL LETTER ZE
      MapItem {0xDBu, UnicodeCodepoint(0x0448u)}, //	CYRILLIC SMALL LETTER SHA
      MapItem {0xDCu, UnicodeCodepoint(0x044Du)}, //	CYRILLIC SMALL LETTER E
      MapItem {0xDDu, UnicodeCodepoint(0x0449u)}, //	CYRILLIC SMALL LETTER SHCHA
      MapItem {0xDEu, UnicodeCodepoint(0x0447u)}, //	CYRILLIC SMALL LETTER CHE
      MapItem {0xDFu, UnicodeCodepoint(0x044Au)}, //	CYRILLIC SMALL LETTER HARD SIGN
      MapItem {0xE0u, UnicodeCodepoint(0x042Eu)}, //	CYRILLIC CAPITAL LETTER YU
      MapItem {0xE1u, UnicodeCodepoint(0x0410u)}, //	CYRILLIC CAPITAL LETTER A
      MapItem {0xE2u, UnicodeCodepoint(0x0411u)}, //	CYRILLIC CAPITAL LETTER BE
      MapItem {0xE3u, UnicodeCodepoint(0x0426u)}, //	CYRILLIC CAPITAL LETTER TSE
      MapItem {0xE4u, UnicodeCodepoint(0x0414u)}, //	CYRILLIC CAPITAL LETTER DE
      MapItem {0xE5u, UnicodeCodepoint(0x0415u)}, //	CYRILLIC CAPITAL LETTER IE
      MapItem {0xE6u, UnicodeCodepoint(0x0424u)}, //	CYRILLIC CAPITAL LETTER EF
      MapItem {0xE7u, UnicodeCodepoint(0x0413u)}, //	CYRILLIC CAPITAL LETTER GHE
      MapItem {0xE8u, UnicodeCodepoint(0x0425u)}, //	CYRILLIC CAPITAL LETTER HA
      MapItem {0xE9u, UnicodeCodepoint(0x0418u)}, //	CYRILLIC CAPITAL LETTER I
      MapItem {0xEAu, UnicodeCodepoint(0x0419u)}, //	CYRILLIC CAPITAL LETTER SHORT I
      MapItem {0xEBu, UnicodeCodepoint(0x041Au)}, //	CYRILLIC CAPITAL LETTER KA
      MapItem {0xECu, UnicodeCodepoint(0x041Bu)}, //	CYRILLIC CAPITAL LETTER EL
      MapItem {0xEDu, UnicodeCodepoint(0x041Cu)}, //	CYRILLIC CAPITAL LETTER EM
      MapItem {0xEEu, UnicodeCodepoint(0x041Du)}, //	CYRILLIC CAPITAL LETTER EN
      MapItem {0xEFu, UnicodeCodepoint(0x041Eu)}, //	CYRILLIC CAPITAL LETTER O
      MapItem {0xF0u, UnicodeCodepoint(0x041Fu)}, //	CYRILLIC CAPITAL LETTER PE
      MapItem {0xF1u, UnicodeCodepoint(0x042Fu)}, //	CYRILLIC CAPITAL LETTER YA
      MapItem {0xF2u, UnicodeCodepoint(0x0420u)}, //	CYRILLIC CAPITAL LETTER ER
      MapItem {0xF3u, UnicodeCodepoint(0x0421u)}, //	CYRILLIC CAPITAL LETTER ES
      MapItem {0xF4u, UnicodeCodepoint(0x0422u)}, //	CYRILLIC CAPITAL LETTER TE
      MapItem {0xF5u, UnicodeCodepoint(0x0423u)}, //	CYRILLIC CAPITAL LETTER U
      MapItem {0xF6u, UnicodeCodepoint(0x0416u)}, //	CYRILLIC CAPITAL LETTER ZHE
      MapItem {0xF7u, UnicodeCodepoint(0x0412u)}, //	CYRILLIC CAPITAL LETTER VE
      MapItem {0xF8u, UnicodeCodepoint(0x042Cu)}, //	CYRILLIC CAPITAL LETTER SOFT SIGN
      MapItem {0xF9u, UnicodeCodepoint(0x042Bu)}, //	CYRILLIC CAPITAL LETTER YERU
      MapItem {0xFAu, UnicodeCodepoint(0x0417u)}, //	CYRILLIC CAPITAL LETTER ZE
      MapItem {0xFBu, UnicodeCodepoint(0x0428u)}, //	CYRILLIC CAPITAL LETTER SHA
      MapItem {0xFCu, UnicodeCodepoint(0x042Du)}, //	CYRILLIC CAPITAL LETTER E
      MapItem {0xFDu, UnicodeCodepoint(0x0429u)}, //	CYRILLIC CAPITAL LETTER SHCHA
      MapItem {0xFEu, UnicodeCodepoint(0x0427u)}, //	CYRILLIC CAPITAL LETTER CHE
      MapItem {0xFFu, UnicodeCodepoint(0x042Au)}, //	CYRILLIC CAPITAL LETTER HARD SIGN
    };

  public:
    KOI8_U_Encoding() noexcept : SingleByteEncoding(EncodingInformation, LookupTable(LookupMapping))
    {
    }

    ~KOI8_U_Encoding() noexcept override = default;
  };
}