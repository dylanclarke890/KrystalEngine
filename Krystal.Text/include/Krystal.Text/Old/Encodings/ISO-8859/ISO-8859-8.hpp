#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class ISO_8859_8_Encoding : public SingleByteEncoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"ISO-8859-8",
        .Aliases = {u8"iso-8859-8", u8"csiso88598e", u8"csisolatinhebrew", u8"hebrew", u8"iso-8859-8-e",
                    u8"iso-ir-138", u8"iso8859-8", u8"iso88598", u8"iso_8859-8", u8"iso_8859-8:1988",
                    u8"visual"},
        .MIBenum = MIBenum {11u},
        .Win32CodePage = Win32CodePage {28'598u},
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
      MapItem {0xA2u, Rune(0x00A2u)}, //	CENT SIGN
      MapItem {0xA3u, Rune(0x00A3u)}, //	POUND SIGN
      MapItem {0xA4u, Rune(0x00A4u)}, //	CURRENCY SIGN
      MapItem {0xA5u, Rune(0x00A5u)}, //	YEN SIGN
      MapItem {0xA6u, Rune(0x00A6u)}, //	BROKEN BAR
      MapItem {0xA7u, Rune(0x00A7u)}, //	SECTION SIGN
      MapItem {0xA8u, Rune(0x00A8u)}, //	DIAERESIS
      MapItem {0xA9u, Rune(0x00A9u)}, //	COPYRIGHT SIGN
      MapItem {0xAAu, Rune(0x00D7u)}, //	MULTIPLICATION SIGN
      MapItem {0xABu, Rune(0x00ABu)}, //	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, Rune(0x00ACu)}, //	NOT SIGN
      MapItem {0xADu, Rune(0x00ADu)}, //	SOFT HYPHEN
      MapItem {0xAEu, Rune(0x00AEu)}, //	REGISTERED SIGN
      MapItem {0xAFu, Rune(0x00AFu)}, //	MACRON
      MapItem {0xB0u, Rune(0x00B0u)}, //	DEGREE SIGN
      MapItem {0xB1u, Rune(0x00B1u)}, //	PLUS-MINUS SIGN
      MapItem {0xB2u, Rune(0x00B2u)}, //	SUPERSCRIPT TWO
      MapItem {0xB3u, Rune(0x00B3u)}, //	SUPERSCRIPT THREE
      MapItem {0xB4u, Rune(0x00B4u)}, //	ACUTE ACCENT
      MapItem {0xB5u, Rune(0x00B5u)}, //	MICRO SIGN
      MapItem {0xB6u, Rune(0x00B6u)}, //	PILCROW SIGN
      MapItem {0xB7u, Rune(0x00B7u)}, //	MIDDLE DOT
      MapItem {0xB8u, Rune(0x00B8u)}, //	CEDILLA
      MapItem {0xB9u, Rune(0x00B9u)}, //	SUPERSCRIPT ONE
      MapItem {0xBAu, Rune(0x00F7u)}, //	DIVISION SIGN
      MapItem {0xBBu, Rune(0x00BBu)}, //	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, Rune(0x00BCu)}, //	VULGAR FRACTION ONE QUARTER
      MapItem {0xBDu, Rune(0x00BDu)}, //	VULGAR FRACTION ONE HALF
      MapItem {0xBEu, Rune(0x00BEu)}, //	VULGAR FRACTION THREE QUARTERS
      MapItem {0xDFu, Rune(0x2017u)}, //	DOUBLE LOW LINE
      MapItem {0xE0u, Rune(0x05D0u)}, //	HEBREW LETTER ALEF
      MapItem {0xE1u, Rune(0x05D1u)}, //	HEBREW LETTER BET
      MapItem {0xE2u, Rune(0x05D2u)}, //	HEBREW LETTER GIMEL
      MapItem {0xE3u, Rune(0x05D3u)}, //	HEBREW LETTER DALET
      MapItem {0xE4u, Rune(0x05D4u)}, //	HEBREW LETTER HE
      MapItem {0xE5u, Rune(0x05D5u)}, //	HEBREW LETTER VAV
      MapItem {0xE6u, Rune(0x05D6u)}, //	HEBREW LETTER ZAYIN
      MapItem {0xE7u, Rune(0x05D7u)}, //	HEBREW LETTER HET
      MapItem {0xE8u, Rune(0x05D8u)}, //	HEBREW LETTER TET
      MapItem {0xE9u, Rune(0x05D9u)}, //	HEBREW LETTER YOD
      MapItem {0xEAu, Rune(0x05DAu)}, //	HEBREW LETTER FINAL KAF
      MapItem {0xEBu, Rune(0x05DBu)}, //	HEBREW LETTER KAF
      MapItem {0xECu, Rune(0x05DCu)}, //	HEBREW LETTER LAMED
      MapItem {0xEDu, Rune(0x05DDu)}, //	HEBREW LETTER FINAL MEM
      MapItem {0xEEu, Rune(0x05DEu)}, //	HEBREW LETTER MEM
      MapItem {0xEFu, Rune(0x05DFu)}, //	HEBREW LETTER FINAL NUN
      MapItem {0xF0u, Rune(0x05E0u)}, //	HEBREW LETTER NUN
      MapItem {0xF1u, Rune(0x05E1u)}, //	HEBREW LETTER SAMEKH
      MapItem {0xF2u, Rune(0x05E2u)}, //	HEBREW LETTER AYIN
      MapItem {0xF3u, Rune(0x05E3u)}, //	HEBREW LETTER FINAL PE
      MapItem {0xF4u, Rune(0x05E4u)}, //	HEBREW LETTER PE
      MapItem {0xF5u, Rune(0x05E5u)}, //	HEBREW LETTER FINAL TSADI
      MapItem {0xF6u, Rune(0x05E6u)}, //	HEBREW LETTER TSADI
      MapItem {0xF7u, Rune(0x05E7u)}, //	HEBREW LETTER QOF
      MapItem {0xF8u, Rune(0x05E8u)}, //	HEBREW LETTER RESH
      MapItem {0xF9u, Rune(0x05E9u)}, //	HEBREW LETTER SHIN
      MapItem {0xFAu, Rune(0x05EAu)}, //	HEBREW LETTER TAV
      MapItem {0xFDu, Rune(0x200Eu)}, //	LEFT-TO-RIGHT MARK
      MapItem {0xFEu, Rune(0x200Fu)}, //	RIGHT-TO-LEFT MARK
    };

  public:
    ISO_8859_8_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_8_Encoding() noexcept override = default;
  };
}