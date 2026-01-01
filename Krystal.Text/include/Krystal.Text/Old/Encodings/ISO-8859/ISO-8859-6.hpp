#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class ISO_8859_6_Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"ISO-8859-6",
        .Aliases = {u8"iso-8859-6", u8"arabic", u8"asmo-708", u8"csiso88596e", u8"csiso88596i",
                    u8"csisolatinarabic", u8"ecma-114", u8"iso-8859-6-e", u8"iso-8859-6-i", u8"iso-ir-127",
                    u8"iso8859-6", u8"iso88596", u8"iso_8859-6", u8"iso_8859-6:1987"},
        .MIBenum = MIBenum {9u},
        .Win32CodePage = Win32CodePage {28'596u},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, Rune(0x0080u)}, //	<control>
      MapItem {0x81u, Rune(0x0081u)}, //	<control>
      MapItem {0x82u, Rune(0x0082u)}, //	<control>
      MapItem {0x83u, Rune(0x0083u)}, //	<control>
      MapItem {0x84u, Rune(0x0084u)}, //	<control>
      MapItem {0x85u, Rune(0x0085u)}, //	<control>
      MapItem {0x86u, Rune(0x0086u)}, //	<control>
      MapItem {0x87u, Rune(0x0087u)}, //	<control>
      MapItem {0x88u, Rune(0x0088u)}, //	<control>
      MapItem {0x89u, Rune(0x0089u)}, //	<control>
      MapItem {0x8Au, Rune(0x008Au)}, //	<control>
      MapItem {0x8Bu, Rune(0x008Bu)}, //	<control>
      MapItem {0x8Cu, Rune(0x008Cu)}, //	<control>
      MapItem {0x8Du, Rune(0x008Du)}, //	<control>
      MapItem {0x8Eu, Rune(0x008Eu)}, //	<control>
      MapItem {0x8Fu, Rune(0x008Fu)}, //	<control>
      MapItem {0x90u, Rune(0x0090u)}, //	<control>
      MapItem {0x91u, Rune(0x0091u)}, //	<control>
      MapItem {0x92u, Rune(0x0092u)}, //	<control>
      MapItem {0x93u, Rune(0x0093u)}, //	<control>
      MapItem {0x94u, Rune(0x0094u)}, //	<control>
      MapItem {0x95u, Rune(0x0095u)}, //	<control>
      MapItem {0x96u, Rune(0x0096u)}, //	<control>
      MapItem {0x97u, Rune(0x0097u)}, //	<control>
      MapItem {0x98u, Rune(0x0098u)}, //	<control>
      MapItem {0x99u, Rune(0x0099u)}, //	<control>
      MapItem {0x9Au, Rune(0x009Au)}, //	<control>
      MapItem {0x9Bu, Rune(0x009Bu)}, //	<control>
      MapItem {0x9Cu, Rune(0x009Cu)}, //	<control>
      MapItem {0x9Du, Rune(0x009Du)}, //	<control>
      MapItem {0x9Eu, Rune(0x009Eu)}, //	<control>
      MapItem {0x9Fu, Rune(0x009Fu)}, //	<control>
      MapItem {0xA0u, Rune(0x00A0u)}, //	NO-BREAK SPACE
      MapItem {0xA4u, Rune(0x00A4u)}, //	CURRENCY SIGN
      MapItem {0xACu, Rune(0x060Cu)}, //	ARABIC COMMA
      MapItem {0xADu, Rune(0x00ADu)}, //	SOFT HYPHEN
      MapItem {0xBBu, Rune(0x061Bu)}, //	ARABIC SEMICOLON
      MapItem {0xBFu, Rune(0x061Fu)}, //	ARABIC QUESTION MARK
      MapItem {0xC1u, Rune(0x0621u)}, //	ARABIC LETTER HAMZA
      MapItem {0xC2u, Rune(0x0622u)}, //	ARABIC LETTER ALEF WITH MADDA ABOVE
      MapItem {0xC3u, Rune(0x0623u)}, //	ARABIC LETTER ALEF WITH HAMZA ABOVE
      MapItem {0xC4u, Rune(0x0624u)}, //	ARABIC LETTER WAW WITH HAMZA ABOVE
      MapItem {0xC5u, Rune(0x0625u)}, //	ARABIC LETTER ALEF WITH HAMZA BELOW
      MapItem {0xC6u, Rune(0x0626u)}, //	ARABIC LETTER YEH WITH HAMZA ABOVE
      MapItem {0xC7u, Rune(0x0627u)}, //	ARABIC LETTER ALEF
      MapItem {0xC8u, Rune(0x0628u)}, //	ARABIC LETTER BEH
      MapItem {0xC9u, Rune(0x0629u)}, //	ARABIC LETTER TEH MARBUTA
      MapItem {0xCAu, Rune(0x062Au)}, //	ARABIC LETTER TEH
      MapItem {0xCBu, Rune(0x062Bu)}, //	ARABIC LETTER THEH
      MapItem {0xCCu, Rune(0x062Cu)}, //	ARABIC LETTER JEEM
      MapItem {0xCDu, Rune(0x062Du)}, //	ARABIC LETTER HAH
      MapItem {0xCEu, Rune(0x062Eu)}, //	ARABIC LETTER KHAH
      MapItem {0xCFu, Rune(0x062Fu)}, //	ARABIC LETTER DAL
      MapItem {0xD0u, Rune(0x0630u)}, //	ARABIC LETTER THAL
      MapItem {0xD1u, Rune(0x0631u)}, //	ARABIC LETTER REH
      MapItem {0xD2u, Rune(0x0632u)}, //	ARABIC LETTER ZAIN
      MapItem {0xD3u, Rune(0x0633u)}, //	ARABIC LETTER SEEN
      MapItem {0xD4u, Rune(0x0634u)}, //	ARABIC LETTER SHEEN
      MapItem {0xD5u, Rune(0x0635u)}, //	ARABIC LETTER SAD
      MapItem {0xD6u, Rune(0x0636u)}, //	ARABIC LETTER DAD
      MapItem {0xD7u, Rune(0x0637u)}, //	ARABIC LETTER TAH
      MapItem {0xD8u, Rune(0x0638u)}, //	ARABIC LETTER ZAH
      MapItem {0xD9u, Rune(0x0639u)}, //	ARABIC LETTER AIN
      MapItem {0xDAu, Rune(0x063Au)}, //	ARABIC LETTER GHAIN
      MapItem {0xE0u, Rune(0x0640u)}, //	ARABIC TATWEEL
      MapItem {0xE1u, Rune(0x0641u)}, //	ARABIC LETTER FEH
      MapItem {0xE2u, Rune(0x0642u)}, //	ARABIC LETTER QAF
      MapItem {0xE3u, Rune(0x0643u)}, //	ARABIC LETTER KAF
      MapItem {0xE4u, Rune(0x0644u)}, //	ARABIC LETTER LAM
      MapItem {0xE5u, Rune(0x0645u)}, //	ARABIC LETTER MEEM
      MapItem {0xE6u, Rune(0x0646u)}, //	ARABIC LETTER NOON
      MapItem {0xE7u, Rune(0x0647u)}, //	ARABIC LETTER HEH
      MapItem {0xE8u, Rune(0x0648u)}, //	ARABIC LETTER WAW
      MapItem {0xE9u, Rune(0x0649u)}, //	ARABIC LETTER ALEF MAKSURA
      MapItem {0xEAu, Rune(0x064Au)}, //	ARABIC LETTER YEH
      MapItem {0xEBu, Rune(0x064Bu)}, //	ARABIC FATHATAN
      MapItem {0xECu, Rune(0x064Cu)}, //	ARABIC DAMMATAN
      MapItem {0xEDu, Rune(0x064Du)}, //	ARABIC KASRATAN
      MapItem {0xEEu, Rune(0x064Eu)}, //	ARABIC FATHA
      MapItem {0xEFu, Rune(0x064Fu)}, //	ARABIC DAMMA
      MapItem {0xF0u, Rune(0x0650u)}, //	ARABIC KASRA
      MapItem {0xF1u, Rune(0x0651u)}, //	ARABIC SHADDA
      MapItem {0xF2u, Rune(0x0652u)}, //	ARABIC SUKUN
    };

  public:
    ISO_8859_6_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_6_Encoding() noexcept override = default;
  };
}