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
      MapItem {0xA2u, UnicodeCodepoint(0x00A2u)}, //	CENT SIGN
      MapItem {0xA3u, UnicodeCodepoint(0x00A3u)}, //	POUND SIGN
      MapItem {0xA4u, UnicodeCodepoint(0x00A4u)}, //	CURRENCY SIGN
      MapItem {0xA5u, UnicodeCodepoint(0x00A5u)}, //	YEN SIGN
      MapItem {0xA6u, UnicodeCodepoint(0x00A6u)}, //	BROKEN BAR
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, //	SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x00A8u)}, //	DIAERESIS
      MapItem {0xA9u, UnicodeCodepoint(0x00A9u)}, //	COPYRIGHT SIGN
      MapItem {0xAAu, UnicodeCodepoint(0x00D7u)}, //	MULTIPLICATION SIGN
      MapItem {0xABu, UnicodeCodepoint(0x00ABu)}, //	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, UnicodeCodepoint(0x00ACu)}, //	NOT SIGN
      MapItem {0xADu, UnicodeCodepoint(0x00ADu)}, //	SOFT HYPHEN
      MapItem {0xAEu, UnicodeCodepoint(0x00AEu)}, //	REGISTERED SIGN
      MapItem {0xAFu, UnicodeCodepoint(0x00AFu)}, //	MACRON
      MapItem {0xB0u, UnicodeCodepoint(0x00B0u)}, //	DEGREE SIGN
      MapItem {0xB1u, UnicodeCodepoint(0x00B1u)}, //	PLUS-MINUS SIGN
      MapItem {0xB2u, UnicodeCodepoint(0x00B2u)}, //	SUPERSCRIPT TWO
      MapItem {0xB3u, UnicodeCodepoint(0x00B3u)}, //	SUPERSCRIPT THREE
      MapItem {0xB4u, UnicodeCodepoint(0x00B4u)}, //	ACUTE ACCENT
      MapItem {0xB5u, UnicodeCodepoint(0x00B5u)}, //	MICRO SIGN
      MapItem {0xB6u, UnicodeCodepoint(0x00B6u)}, //	PILCROW SIGN
      MapItem {0xB7u, UnicodeCodepoint(0x00B7u)}, //	MIDDLE DOT
      MapItem {0xB8u, UnicodeCodepoint(0x00B8u)}, //	CEDILLA
      MapItem {0xB9u, UnicodeCodepoint(0x00B9u)}, //	SUPERSCRIPT ONE
      MapItem {0xBAu, UnicodeCodepoint(0x00F7u)}, //	DIVISION SIGN
      MapItem {0xBBu, UnicodeCodepoint(0x00BBu)}, //	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, UnicodeCodepoint(0x00BCu)}, //	VULGAR FRACTION ONE QUARTER
      MapItem {0xBDu, UnicodeCodepoint(0x00BDu)}, //	VULGAR FRACTION ONE HALF
      MapItem {0xBEu, UnicodeCodepoint(0x00BEu)}, //	VULGAR FRACTION THREE QUARTERS
      MapItem {0xDFu, UnicodeCodepoint(0x2017u)}, //	DOUBLE LOW LINE
      MapItem {0xE0u, UnicodeCodepoint(0x05D0u)}, //	HEBREW LETTER ALEF
      MapItem {0xE1u, UnicodeCodepoint(0x05D1u)}, //	HEBREW LETTER BET
      MapItem {0xE2u, UnicodeCodepoint(0x05D2u)}, //	HEBREW LETTER GIMEL
      MapItem {0xE3u, UnicodeCodepoint(0x05D3u)}, //	HEBREW LETTER DALET
      MapItem {0xE4u, UnicodeCodepoint(0x05D4u)}, //	HEBREW LETTER HE
      MapItem {0xE5u, UnicodeCodepoint(0x05D5u)}, //	HEBREW LETTER VAV
      MapItem {0xE6u, UnicodeCodepoint(0x05D6u)}, //	HEBREW LETTER ZAYIN
      MapItem {0xE7u, UnicodeCodepoint(0x05D7u)}, //	HEBREW LETTER HET
      MapItem {0xE8u, UnicodeCodepoint(0x05D8u)}, //	HEBREW LETTER TET
      MapItem {0xE9u, UnicodeCodepoint(0x05D9u)}, //	HEBREW LETTER YOD
      MapItem {0xEAu, UnicodeCodepoint(0x05DAu)}, //	HEBREW LETTER FINAL KAF
      MapItem {0xEBu, UnicodeCodepoint(0x05DBu)}, //	HEBREW LETTER KAF
      MapItem {0xECu, UnicodeCodepoint(0x05DCu)}, //	HEBREW LETTER LAMED
      MapItem {0xEDu, UnicodeCodepoint(0x05DDu)}, //	HEBREW LETTER FINAL MEM
      MapItem {0xEEu, UnicodeCodepoint(0x05DEu)}, //	HEBREW LETTER MEM
      MapItem {0xEFu, UnicodeCodepoint(0x05DFu)}, //	HEBREW LETTER FINAL NUN
      MapItem {0xF0u, UnicodeCodepoint(0x05E0u)}, //	HEBREW LETTER NUN
      MapItem {0xF1u, UnicodeCodepoint(0x05E1u)}, //	HEBREW LETTER SAMEKH
      MapItem {0xF2u, UnicodeCodepoint(0x05E2u)}, //	HEBREW LETTER AYIN
      MapItem {0xF3u, UnicodeCodepoint(0x05E3u)}, //	HEBREW LETTER FINAL PE
      MapItem {0xF4u, UnicodeCodepoint(0x05E4u)}, //	HEBREW LETTER PE
      MapItem {0xF5u, UnicodeCodepoint(0x05E5u)}, //	HEBREW LETTER FINAL TSADI
      MapItem {0xF6u, UnicodeCodepoint(0x05E6u)}, //	HEBREW LETTER TSADI
      MapItem {0xF7u, UnicodeCodepoint(0x05E7u)}, //	HEBREW LETTER QOF
      MapItem {0xF8u, UnicodeCodepoint(0x05E8u)}, //	HEBREW LETTER RESH
      MapItem {0xF9u, UnicodeCodepoint(0x05E9u)}, //	HEBREW LETTER SHIN
      MapItem {0xFAu, UnicodeCodepoint(0x05EAu)}, //	HEBREW LETTER TAV
      MapItem {0xFDu, UnicodeCodepoint(0x200Eu)}, //	LEFT-TO-RIGHT MARK
      MapItem {0xFEu, UnicodeCodepoint(0x200Fu)}, //	RIGHT-TO-LEFT MARK
    };

  public:
    ISO_8859_8_Encoding() noexcept : SingleByteEncoding(GetEncodingInfo(), LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_8_Encoding() noexcept override = default;
  };
}