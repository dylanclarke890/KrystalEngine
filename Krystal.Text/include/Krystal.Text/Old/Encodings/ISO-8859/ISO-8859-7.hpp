#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class ISO_8859_7_Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"ISO-8859-7",
        .Aliases = {u8"iso-8859-7", u8"csisolatingreek", u8"ecma-118", u8"elot_928", u8"greek", u8"greek8",
                    u8"iso-ir-126", u8"iso8859-7", u8"iso88597", u8"iso_8859-7", u8"iso_8859-7:1987",
                    u8"sun_eu_greek"},
        .MIBenum = MIBenum {10u},
        .Win32CodePage = Win32CodePage {28'597u},
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
      MapItem {0xA1u, Rune(0x2018u)}, //	LEFT SINGLE QUOTATION MARK
      MapItem {0xA2u, Rune(0x2019u)}, //	RIGHT SINGLE QUOTATION MARK
      MapItem {0xA3u, Rune(0x00A3u)}, //	POUND SIGN
      MapItem {0xA4u, Rune(0x20ACu)}, //	EURO SIGN
      MapItem {0xA5u, Rune(0x20AFu)}, //	DRACHMA SIGN
      MapItem {0xA6u, Rune(0x00A6u)}, //	BROKEN BAR
      MapItem {0xA7u, Rune(0x00A7u)}, //	SECTION SIGN
      MapItem {0xA8u, Rune(0x00A8u)}, //	DIAERESIS
      MapItem {0xA9u, Rune(0x00A9u)}, //	COPYRIGHT SIGN
      MapItem {0xAAu, Rune(0x037Au)}, //	GREEK YPOGEGRAMMENI
      MapItem {0xABu, Rune(0x00ABu)}, //	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, Rune(0x00ACu)}, //	NOT SIGN
      MapItem {0xADu, Rune(0x00ADu)}, //	SOFT HYPHEN
      MapItem {0xAFu, Rune(0x2015u)}, //	HORIZONTAL BAR
      MapItem {0xB0u, Rune(0x00B0u)}, //	DEGREE SIGN
      MapItem {0xB1u, Rune(0x00B1u)}, //	PLUS-MINUS SIGN
      MapItem {0xB2u, Rune(0x00B2u)}, //	SUPERSCRIPT TWO
      MapItem {0xB3u, Rune(0x00B3u)}, //	SUPERSCRIPT THREE
      MapItem {0xB4u, Rune(0x0384u)}, //	GREEK TONOS
      MapItem {0xB5u, Rune(0x0385u)}, //	GREEK DIALYTIKA TONOS
      MapItem {0xB6u, Rune(0x0386u)}, //	GREEK CAPITAL LETTER ALPHA WITH TONOS
      MapItem {0xB7u, Rune(0x00B7u)}, //	MIDDLE DOT
      MapItem {0xB8u, Rune(0x0388u)}, //	GREEK CAPITAL LETTER EPSILON WITH TONOS
      MapItem {0xB9u, Rune(0x0389u)}, //	GREEK CAPITAL LETTER ETA WITH TONOS
      MapItem {0xBAu, Rune(0x038Au)}, //	GREEK CAPITAL LETTER IOTA WITH TONOS
      MapItem {0xBBu, Rune(0x00BBu)}, //	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, Rune(0x038Cu)}, //	GREEK CAPITAL LETTER OMICRON WITH TONOS
      MapItem {0xBDu, Rune(0x00BDu)}, //	VULGAR FRACTION ONE HALF
      MapItem {0xBEu, Rune(0x038Eu)}, //	GREEK CAPITAL LETTER UPSILON WITH TONOS
      MapItem {0xBFu, Rune(0x038Fu)}, //	GREEK CAPITAL LETTER OMEGA WITH TONOS
      MapItem {0xC0u, Rune(0x0390u)}, //	GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
      MapItem {0xC1u, Rune(0x0391u)}, //	GREEK CAPITAL LETTER ALPHA
      MapItem {0xC2u, Rune(0x0392u)}, //	GREEK CAPITAL LETTER BETA
      MapItem {0xC3u, Rune(0x0393u)}, //	GREEK CAPITAL LETTER GAMMA
      MapItem {0xC4u, Rune(0x0394u)}, //	GREEK CAPITAL LETTER DELTA
      MapItem {0xC5u, Rune(0x0395u)}, //	GREEK CAPITAL LETTER EPSILON
      MapItem {0xC6u, Rune(0x0396u)}, //	GREEK CAPITAL LETTER ZETA
      MapItem {0xC7u, Rune(0x0397u)}, //	GREEK CAPITAL LETTER ETA
      MapItem {0xC8u, Rune(0x0398u)}, //	GREEK CAPITAL LETTER THETA
      MapItem {0xC9u, Rune(0x0399u)}, //	GREEK CAPITAL LETTER IOTA
      MapItem {0xCAu, Rune(0x039Au)}, //	GREEK CAPITAL LETTER KAPPA
      MapItem {0xCBu, Rune(0x039Bu)}, //	GREEK CAPITAL LETTER LAMDA
      MapItem {0xCCu, Rune(0x039Cu)}, //	GREEK CAPITAL LETTER MU
      MapItem {0xCDu, Rune(0x039Du)}, //	GREEK CAPITAL LETTER NU
      MapItem {0xCEu, Rune(0x039Eu)}, //	GREEK CAPITAL LETTER XI
      MapItem {0xCFu, Rune(0x039Fu)}, //	GREEK CAPITAL LETTER OMICRON
      MapItem {0xD0u, Rune(0x03A0u)}, //	GREEK CAPITAL LETTER PI
      MapItem {0xD1u, Rune(0x03A1u)}, //	GREEK CAPITAL LETTER RHO
      MapItem {0xD3u, Rune(0x03A3u)}, //	GREEK CAPITAL LETTER SIGMA
      MapItem {0xD4u, Rune(0x03A4u)}, //	GREEK CAPITAL LETTER TAU
      MapItem {0xD5u, Rune(0x03A5u)}, //	GREEK CAPITAL LETTER UPSILON
      MapItem {0xD6u, Rune(0x03A6u)}, //	GREEK CAPITAL LETTER PHI
      MapItem {0xD7u, Rune(0x03A7u)}, //	GREEK CAPITAL LETTER CHI
      MapItem {0xD8u, Rune(0x03A8u)}, //	GREEK CAPITAL LETTER PSI
      MapItem {0xD9u, Rune(0x03A9u)}, //	GREEK CAPITAL LETTER OMEGA
      MapItem {0xDAu, Rune(0x03AAu)}, //	GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
      MapItem {0xDBu, Rune(0x03ABu)}, //	GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
      MapItem {0xDCu, Rune(0x03ACu)}, //	GREEK SMALL LETTER ALPHA WITH TONOS
      MapItem {0xDDu, Rune(0x03ADu)}, //	GREEK SMALL LETTER EPSILON WITH TONOS
      MapItem {0xDEu, Rune(0x03AEu)}, //	GREEK SMALL LETTER ETA WITH TONOS
      MapItem {0xDFu, Rune(0x03AFu)}, //	GREEK SMALL LETTER IOTA WITH TONOS
      MapItem {0xE0u, Rune(0x03B0u)}, //	GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
      MapItem {0xE1u, Rune(0x03B1u)}, //	GREEK SMALL LETTER ALPHA
      MapItem {0xE2u, Rune(0x03B2u)}, //	GREEK SMALL LETTER BETA
      MapItem {0xE3u, Rune(0x03B3u)}, //	GREEK SMALL LETTER GAMMA
      MapItem {0xE4u, Rune(0x03B4u)}, //	GREEK SMALL LETTER DELTA
      MapItem {0xE5u, Rune(0x03B5u)}, //	GREEK SMALL LETTER EPSILON
      MapItem {0xE6u, Rune(0x03B6u)}, //	GREEK SMALL LETTER ZETA
      MapItem {0xE7u, Rune(0x03B7u)}, //	GREEK SMALL LETTER ETA
      MapItem {0xE8u, Rune(0x03B8u)}, //	GREEK SMALL LETTER THETA
      MapItem {0xE9u, Rune(0x03B9u)}, //	GREEK SMALL LETTER IOTA
      MapItem {0xEAu, Rune(0x03BAu)}, //	GREEK SMALL LETTER KAPPA
      MapItem {0xEBu, Rune(0x03BBu)}, //	GREEK SMALL LETTER LAMDA
      MapItem {0xECu, Rune(0x03BCu)}, //	GREEK SMALL LETTER MU
      MapItem {0xEDu, Rune(0x03BDu)}, //	GREEK SMALL LETTER NU
      MapItem {0xEEu, Rune(0x03BEu)}, //	GREEK SMALL LETTER XI
      MapItem {0xEFu, Rune(0x03BFu)}, //	GREEK SMALL LETTER OMICRON
      MapItem {0xF0u, Rune(0x03C0u)}, //	GREEK SMALL LETTER PI
      MapItem {0xF1u, Rune(0x03C1u)}, //	GREEK SMALL LETTER RHO
      MapItem {0xF2u, Rune(0x03C2u)}, //	GREEK SMALL LETTER FINAL SIGMA
      MapItem {0xF3u, Rune(0x03C3u)}, //	GREEK SMALL LETTER SIGMA
      MapItem {0xF4u, Rune(0x03C4u)}, //	GREEK SMALL LETTER TAU
      MapItem {0xF5u, Rune(0x03C5u)}, //	GREEK SMALL LETTER UPSILON
      MapItem {0xF6u, Rune(0x03C6u)}, //	GREEK SMALL LETTER PHI
      MapItem {0xF7u, Rune(0x03C7u)}, //	GREEK SMALL LETTER CHI
      MapItem {0xF8u, Rune(0x03C8u)}, //	GREEK SMALL LETTER PSI
      MapItem {0xF9u, Rune(0x03C9u)}, //	GREEK SMALL LETTER OMEGA
      MapItem {0xFAu, Rune(0x03CAu)}, //	GREEK SMALL LETTER IOTA WITH DIALYTIKA
      MapItem {0xFBu, Rune(0x03CBu)}, //	GREEK SMALL LETTER UPSILON WITH DIALYTIKA
      MapItem {0xFCu, Rune(0x03CCu)}, //	GREEK SMALL LETTER OMICRON WITH TONOS
      MapItem {0xFDu, Rune(0x03CDu)}, //	GREEK SMALL LETTER UPSILON WITH TONOS
      MapItem {0xFEu, Rune(0x03CEu)}, //	GREEK SMALL LETTER OMEGA WITH TONOS
    };

  public:
    ISO_8859_7_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_7_Encoding() noexcept override = default;
  };
}