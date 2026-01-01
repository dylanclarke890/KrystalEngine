#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class ISO_8859_5_Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"ISO-8859-5",
        .Aliases = {u8"csisolatincyrillic", u8"cyrillic", u8"iso-ir-144", u8"iso8859-5", u8"iso88595",
                    u8"iso_8859-5", u8"iso_8859-5:1988", u8"iso-8859-5"},
        .MIBenum = MIBenum {8u},
        .Win32CodePage = Win32CodePage {28'595u},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, Rune(0x0080u)}, //<control>
      MapItem {0x81u, Rune(0x0081u)}, //<control>
      MapItem {0x82u, Rune(0x0082u)}, //<control>
      MapItem {0x83u, Rune(0x0083u)}, //<control>
      MapItem {0x84u, Rune(0x0084u)}, //<control>
      MapItem {0x85u, Rune(0x0085u)}, //<control>
      MapItem {0x86u, Rune(0x0086u)}, //<control>
      MapItem {0x87u, Rune(0x0087u)}, //<control>
      MapItem {0x88u, Rune(0x0088u)}, //<control>
      MapItem {0x89u, Rune(0x0089u)}, //<control>
      MapItem {0x8Au, Rune(0x008Au)}, //<control>
      MapItem {0x8Bu, Rune(0x008Bu)}, //<control>
      MapItem {0x8Cu, Rune(0x008Cu)}, //<control>
      MapItem {0x8Du, Rune(0x008Du)}, //<control>
      MapItem {0x8Eu, Rune(0x008Eu)}, //<control>
      MapItem {0x8Fu, Rune(0x008Fu)}, //<control>
      MapItem {0x90u, Rune(0x0090u)}, //<control>
      MapItem {0x91u, Rune(0x0091u)}, //<control>
      MapItem {0x92u, Rune(0x0092u)}, //<control>
      MapItem {0x93u, Rune(0x0093u)}, //<control>
      MapItem {0x94u, Rune(0x0094u)}, //<control>
      MapItem {0x95u, Rune(0x0095u)}, //<control>
      MapItem {0x96u, Rune(0x0096u)}, //<control>
      MapItem {0x97u, Rune(0x0097u)}, //<control>
      MapItem {0x98u, Rune(0x0098u)}, //<control>
      MapItem {0x99u, Rune(0x0099u)}, //<control>
      MapItem {0x9Au, Rune(0x009Au)}, //<control>
      MapItem {0x9Bu, Rune(0x009Bu)}, //<control>
      MapItem {0x9Cu, Rune(0x009Cu)}, //<control>
      MapItem {0x9Du, Rune(0x009Du)}, //<control>
      MapItem {0x9Eu, Rune(0x009Eu)}, //<control>
      MapItem {0x9Fu, Rune(0x009Fu)}, //<control>
      MapItem {0xA0u, Rune(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, Rune(0x0401u)}, // CYRILLIC CAPITAL LETTER IO
      MapItem {0xA2u, Rune(0x0402u)}, // CYRILLIC CAPITAL LETTER DJE
      MapItem {0xA3u, Rune(0x0403u)}, // CYRILLIC CAPITAL LETTER GJE
      MapItem {0xA4u, Rune(0x0404u)}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
      MapItem {0xA5u, Rune(0x0405u)}, // CYRILLIC CAPITAL LETTER DZE
      MapItem {0xA6u, Rune(0x0406u)}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
      MapItem {0xA7u, Rune(0x0407u)}, // CYRILLIC CAPITAL LETTER YI
      MapItem {0xA8u, Rune(0x0408u)}, // CYRILLIC CAPITAL LETTER JE
      MapItem {0xA9u, Rune(0x0409u)}, // CYRILLIC CAPITAL LETTER LJE
      MapItem {0xAAu, Rune(0x040Au)}, // CYRILLIC CAPITAL LETTER NJE
      MapItem {0xABu, Rune(0x040Bu)}, // CYRILLIC CAPITAL LETTER TSHE
      MapItem {0xACu, Rune(0x040Cu)}, // CYRILLIC CAPITAL LETTER KJE
      MapItem {0xADu, Rune(0x00ADu)}, // SOFT HYPHEN
      MapItem {0xAEu, Rune(0x040Eu)}, // CYRILLIC CAPITAL LETTER SHORT U
      MapItem {0xAFu, Rune(0x040Fu)}, // CYRILLIC CAPITAL LETTER DZHE
      MapItem {0xB0u, Rune(0x0410u)}, // CYRILLIC CAPITAL LETTER A
      MapItem {0xB1u, Rune(0x0411u)}, // CYRILLIC CAPITAL LETTER BE
      MapItem {0xB2u, Rune(0x0412u)}, // CYRILLIC CAPITAL LETTER VE
      MapItem {0xB3u, Rune(0x0413u)}, // CYRILLIC CAPITAL LETTER GHE
      MapItem {0xB4u, Rune(0x0414u)}, // CYRILLIC CAPITAL LETTER DE
      MapItem {0xB5u, Rune(0x0415u)}, // CYRILLIC CAPITAL LETTER IE
      MapItem {0xB6u, Rune(0x0416u)}, // CYRILLIC CAPITAL LETTER ZHE
      MapItem {0xB7u, Rune(0x0417u)}, // CYRILLIC CAPITAL LETTER ZE
      MapItem {0xB8u, Rune(0x0418u)}, // CYRILLIC CAPITAL LETTER I
      MapItem {0xB9u, Rune(0x0419u)}, // CYRILLIC CAPITAL LETTER SHORT I
      MapItem {0xBAu, Rune(0x041Au)}, // CYRILLIC CAPITAL LETTER KA
      MapItem {0xBBu, Rune(0x041Bu)}, // CYRILLIC CAPITAL LETTER EL
      MapItem {0xBCu, Rune(0x041Cu)}, // CYRILLIC CAPITAL LETTER EM
      MapItem {0xBDu, Rune(0x041Du)}, // CYRILLIC CAPITAL LETTER EN
      MapItem {0xBEu, Rune(0x041Eu)}, // CYRILLIC CAPITAL LETTER O
      MapItem {0xBFu, Rune(0x041Fu)}, // CYRILLIC CAPITAL LETTER PE
      MapItem {0xC0u, Rune(0x0420u)}, // CYRILLIC CAPITAL LETTER ER
      MapItem {0xC1u, Rune(0x0421u)}, // CYRILLIC CAPITAL LETTER ES
      MapItem {0xC2u, Rune(0x0422u)}, // CYRILLIC CAPITAL LETTER TE
      MapItem {0xC3u, Rune(0x0423u)}, // CYRILLIC CAPITAL LETTER U
      MapItem {0xC4u, Rune(0x0424u)}, // CYRILLIC CAPITAL LETTER EF
      MapItem {0xC5u, Rune(0x0425u)}, // CYRILLIC CAPITAL LETTER HA
      MapItem {0xC6u, Rune(0x0426u)}, // CYRILLIC CAPITAL LETTER TSE
      MapItem {0xC7u, Rune(0x0427u)}, // CYRILLIC CAPITAL LETTER CHE
      MapItem {0xC8u, Rune(0x0428u)}, // CYRILLIC CAPITAL LETTER SHA
      MapItem {0xC9u, Rune(0x0429u)}, // CYRILLIC CAPITAL LETTER SHCHA
      MapItem {0xCAu, Rune(0x042Au)}, // CYRILLIC CAPITAL LETTER HARD SIGN
      MapItem {0xCBu, Rune(0x042Bu)}, // CYRILLIC CAPITAL LETTER YERU
      MapItem {0xCCu, Rune(0x042Cu)}, // CYRILLIC CAPITAL LETTER SOFT SIGN
      MapItem {0xCDu, Rune(0x042Du)}, // CYRILLIC CAPITAL LETTER E
      MapItem {0xCEu, Rune(0x042Eu)}, // CYRILLIC CAPITAL LETTER YU
      MapItem {0xCFu, Rune(0x042Fu)}, // CYRILLIC CAPITAL LETTER YA
      MapItem {0xD0u, Rune(0x0430u)}, // CYRILLIC SMALL LETTER A
      MapItem {0xD1u, Rune(0x0431u)}, // CYRILLIC SMALL LETTER BE
      MapItem {0xD2u, Rune(0x0432u)}, // CYRILLIC SMALL LETTER VE
      MapItem {0xD3u, Rune(0x0433u)}, // CYRILLIC SMALL LETTER GHE
      MapItem {0xD4u, Rune(0x0434u)}, // CYRILLIC SMALL LETTER DE
      MapItem {0xD5u, Rune(0x0435u)}, // CYRILLIC SMALL LETTER IE
      MapItem {0xD6u, Rune(0x0436u)}, // CYRILLIC SMALL LETTER ZHE
      MapItem {0xD7u, Rune(0x0437u)}, // CYRILLIC SMALL LETTER ZE
      MapItem {0xD8u, Rune(0x0438u)}, // CYRILLIC SMALL LETTER I
      MapItem {0xD9u, Rune(0x0439u)}, // CYRILLIC SMALL LETTER SHORT I
      MapItem {0xDAu, Rune(0x043Au)}, // CYRILLIC SMALL LETTER KA
      MapItem {0xDBu, Rune(0x043Bu)}, // CYRILLIC SMALL LETTER EL
      MapItem {0xDCu, Rune(0x043Cu)}, // CYRILLIC SMALL LETTER EM
      MapItem {0xDDu, Rune(0x043Du)}, // CYRILLIC SMALL LETTER EN
      MapItem {0xDEu, Rune(0x043Eu)}, // CYRILLIC SMALL LETTER O
      MapItem {0xDFu, Rune(0x043Fu)}, // CYRILLIC SMALL LETTER PE
      MapItem {0xE0u, Rune(0x0440u)}, // CYRILLIC SMALL LETTER ER
      MapItem {0xE1u, Rune(0x0441u)}, // CYRILLIC SMALL LETTER ES
      MapItem {0xE2u, Rune(0x0442u)}, // CYRILLIC SMALL LETTER TE
      MapItem {0xE3u, Rune(0x0443u)}, // CYRILLIC SMALL LETTER U
      MapItem {0xE4u, Rune(0x0444u)}, // CYRILLIC SMALL LETTER EF
      MapItem {0xE5u, Rune(0x0445u)}, // CYRILLIC SMALL LETTER HA
      MapItem {0xE6u, Rune(0x0446u)}, // CYRILLIC SMALL LETTER TSE
      MapItem {0xE7u, Rune(0x0447u)}, // CYRILLIC SMALL LETTER CHE
      MapItem {0xE8u, Rune(0x0448u)}, // CYRILLIC SMALL LETTER SHA
      MapItem {0xE9u, Rune(0x0449u)}, // CYRILLIC SMALL LETTER SHCHA
      MapItem {0xEAu, Rune(0x044Au)}, // CYRILLIC SMALL LETTER HARD SIGN
      MapItem {0xEBu, Rune(0x044Bu)}, // CYRILLIC SMALL LETTER YERU
      MapItem {0xECu, Rune(0x044Cu)}, // CYRILLIC SMALL LETTER SOFT SIGN
      MapItem {0xEDu, Rune(0x044Du)}, // CYRILLIC SMALL LETTER E
      MapItem {0xEEu, Rune(0x044Eu)}, // CYRILLIC SMALL LETTER YU
      MapItem {0xEFu, Rune(0x044Fu)}, // CYRILLIC SMALL LETTER YA
      MapItem {0xF0u, Rune(0x2116u)}, // NUMERO SIGN
      MapItem {0xF1u, Rune(0x0451u)}, // CYRILLIC SMALL LETTER IO
      MapItem {0xF2u, Rune(0x0452u)}, // CYRILLIC SMALL LETTER DJE
      MapItem {0xF3u, Rune(0x0453u)}, // CYRILLIC SMALL LETTER GJE
      MapItem {0xF4u, Rune(0x0454u)}, // CYRILLIC SMALL LETTER UKRAINIAN IE
      MapItem {0xF5u, Rune(0x0455u)}, // CYRILLIC SMALL LETTER DZE
      MapItem {0xF6u, Rune(0x0456u)}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
      MapItem {0xF7u, Rune(0x0457u)}, // CYRILLIC SMALL LETTER YI
      MapItem {0xF8u, Rune(0x0458u)}, // CYRILLIC SMALL LETTER JE
      MapItem {0xF9u, Rune(0x0459u)}, // CYRILLIC SMALL LETTER LJE
      MapItem {0xFAu, Rune(0x045Au)}, // CYRILLIC SMALL LETTER NJE
      MapItem {0xFBu, Rune(0x045Bu)}, // CYRILLIC SMALL LETTER TSHE
      MapItem {0xFCu, Rune(0x045Cu)}, // CYRILLIC SMALL LETTER KJE
      MapItem {0xFDu, Rune(0x00A7u)}, // SECTION SIGN
      MapItem {0xFEu, Rune(0x045Eu)}, // CYRILLIC SMALL LETTER SHORT U
      MapItem {0xFFu, Rune(0x045Fu)}, // CYRILLIC SMALL LETTER DZHE
    };

  public:
    ISO_8859_5_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_5_Encoding() noexcept override = default;
  };
}