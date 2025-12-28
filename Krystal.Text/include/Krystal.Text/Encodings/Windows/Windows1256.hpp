#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class Windows1256Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"windows-1256";
    static constexpr uint32 MIBenum = 2'256u;
    static constexpr uint32 WindowsCodePage = 1'256u;

    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, UnicodeCodepoint(0x20ACu)}, // EURO SIGN
      MapItem {0x81u, UnicodeCodepoint(0x067Eu)}, // ARABIC LETTER PEH
      MapItem {0x82u, UnicodeCodepoint(0x201Au)}, // SINGLE LOW-9 QUOTATION MARK
      MapItem {0x83u, UnicodeCodepoint(0x0192u)}, // LATIN SMALL LETTER F WITH HOOK
      MapItem {0x84u, UnicodeCodepoint(0x201Eu)}, // DOUBLE LOW-9 QUOTATION MARK
      MapItem {0x85u, UnicodeCodepoint(0x2026u)}, // HORIZONTAL ELLIPSIS
      MapItem {0x86u, UnicodeCodepoint(0x2020u)}, // DAGGER
      MapItem {0x87u, UnicodeCodepoint(0x2021u)}, // DOUBLE DAGGER
      MapItem {0x88u, UnicodeCodepoint(0x02C6u)}, // MODIFIER LETTER CIRCUMFLEX ACCENT
      MapItem {0x89u, UnicodeCodepoint(0x2030u)}, // PER MILLE SIGN
      MapItem {0x8Au, UnicodeCodepoint(0x0679u)}, // ARABIC LETTER TTEH
      MapItem {0x8Bu, UnicodeCodepoint(0x2039u)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
      MapItem {0x8Cu, UnicodeCodepoint(0x0152u)}, // LATIN CAPITAL LIGATURE OE
      MapItem {0x8Du, UnicodeCodepoint(0x0686u)}, // ARABIC LETTER TCHEH
      MapItem {0x8Eu, UnicodeCodepoint(0x0698u)}, // ARABIC LETTER JEH
      MapItem {0x8Fu, UnicodeCodepoint(0x0688u)}, // ARABIC LETTER DDAL
      MapItem {0x90u, UnicodeCodepoint(0x06AFu)}, // ARABIC LETTER GAF
      MapItem {0x91u, UnicodeCodepoint(0x2018u)}, // LEFT SINGLE QUOTATION MARK
      MapItem {0x92u, UnicodeCodepoint(0x2019u)}, // RIGHT SINGLE QUOTATION MARK
      MapItem {0x93u, UnicodeCodepoint(0x201Cu)}, // LEFT DOUBLE QUOTATION MARK
      MapItem {0x94u, UnicodeCodepoint(0x201Du)}, // RIGHT DOUBLE QUOTATION MARK
      MapItem {0x95u, UnicodeCodepoint(0x2022u)}, // BULLET
      MapItem {0x96u, UnicodeCodepoint(0x2013u)}, // EN DASH
      MapItem {0x97u, UnicodeCodepoint(0x2014u)}, // EM DASH
      MapItem {0x98u, UnicodeCodepoint(0x06A9u)}, // ARABIC LETTER KEHEH
      MapItem {0x99u, UnicodeCodepoint(0x2122u)}, // TRADE MARK SIGN
      MapItem {0x9Au, UnicodeCodepoint(0x0691u)}, // ARABIC LETTER RREH
      MapItem {0x9Bu, UnicodeCodepoint(0x203Au)}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
      MapItem {0x9Cu, UnicodeCodepoint(0x0153u)}, // LATIN SMALL LIGATURE OE
      MapItem {0x9Du, UnicodeCodepoint(0x200Cu)}, // ZERO WIDTH NON-JOINER
      MapItem {0x9Eu, UnicodeCodepoint(0x200Du)}, // ZERO WIDTH JOINER
      MapItem {0x9Fu, UnicodeCodepoint(0x06BAu)}, // ARABIC LETTER NOON GHUNNA
      MapItem {0xA0u, UnicodeCodepoint(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, UnicodeCodepoint(0x060Cu)}, // ARABIC COMMA
      MapItem {0xA2u, UnicodeCodepoint(0x00A2u)}, // CENT SIGN
      MapItem {0xA3u, UnicodeCodepoint(0x00A3u)}, // POUND SIGN
      MapItem {0xA4u, UnicodeCodepoint(0x00A4u)}, // CURRENCY SIGN
      MapItem {0xA5u, UnicodeCodepoint(0x00A5u)}, // YEN SIGN
      MapItem {0xA6u, UnicodeCodepoint(0x00A6u)}, // BROKEN BAR
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, UnicodeCodepoint(0x00A9u)}, // COPYRIGHT SIGN
      MapItem {0xAAu, UnicodeCodepoint(0x06BEu)}, // ARABIC LETTER HEH DOACHASHMEE
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
      MapItem {0xBAu, UnicodeCodepoint(0x061Bu)}, // ARABIC SEMICOLON
      MapItem {0xBBu, UnicodeCodepoint(0x00BBu)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, UnicodeCodepoint(0x00BCu)}, // VULGAR FRACTION ONE QUARTER
      MapItem {0xBDu, UnicodeCodepoint(0x00BDu)}, // VULGAR FRACTION ONE HALF
      MapItem {0xBEu, UnicodeCodepoint(0x00BEu)}, // VULGAR FRACTION THREE QUARTERS
      MapItem {0xBFu, UnicodeCodepoint(0x061Fu)}, // ARABIC QUESTION MARK
      MapItem {0xC0u, UnicodeCodepoint(0x06C1u)}, // ARABIC LETTER HEH GOAL
      MapItem {0xC1u, UnicodeCodepoint(0x0621u)}, // ARABIC LETTER HAMZA
      MapItem {0xC2u, UnicodeCodepoint(0x0622u)}, // ARABIC LETTER ALEF WITH MADDA ABOVE
      MapItem {0xC3u, UnicodeCodepoint(0x0623u)}, // ARABIC LETTER ALEF WITH HAMZA ABOVE
      MapItem {0xC4u, UnicodeCodepoint(0x0624u)}, // ARABIC LETTER WAW WITH HAMZA ABOVE
      MapItem {0xC5u, UnicodeCodepoint(0x0625u)}, // ARABIC LETTER ALEF WITH HAMZA BELOW
      MapItem {0xC6u, UnicodeCodepoint(0x0626u)}, // ARABIC LETTER YEH WITH HAMZA ABOVE
      MapItem {0xC7u, UnicodeCodepoint(0x0627u)}, // ARABIC LETTER ALEF
      MapItem {0xC8u, UnicodeCodepoint(0x0628u)}, // ARABIC LETTER BEH
      MapItem {0xC9u, UnicodeCodepoint(0x0629u)}, // ARABIC LETTER TEH MARBUTA
      MapItem {0xCAu, UnicodeCodepoint(0x062Au)}, // ARABIC LETTER TEH
      MapItem {0xCBu, UnicodeCodepoint(0x062Bu)}, // ARABIC LETTER THEH
      MapItem {0xCCu, UnicodeCodepoint(0x062Cu)}, // ARABIC LETTER JEEM
      MapItem {0xCDu, UnicodeCodepoint(0x062Du)}, // ARABIC LETTER HAH
      MapItem {0xCEu, UnicodeCodepoint(0x062Eu)}, // ARABIC LETTER KHAH
      MapItem {0xCFu, UnicodeCodepoint(0x062Fu)}, // ARABIC LETTER DAL
      MapItem {0xD0u, UnicodeCodepoint(0x0630u)}, // ARABIC LETTER THAL
      MapItem {0xD1u, UnicodeCodepoint(0x0631u)}, // ARABIC LETTER REH
      MapItem {0xD2u, UnicodeCodepoint(0x0632u)}, // ARABIC LETTER ZAIN
      MapItem {0xD3u, UnicodeCodepoint(0x0633u)}, // ARABIC LETTER SEEN
      MapItem {0xD4u, UnicodeCodepoint(0x0634u)}, // ARABIC LETTER SHEEN
      MapItem {0xD5u, UnicodeCodepoint(0x0635u)}, // ARABIC LETTER SAD
      MapItem {0xD6u, UnicodeCodepoint(0x0636u)}, // ARABIC LETTER DAD
      MapItem {0xD7u, UnicodeCodepoint(0x00D7u)}, // MULTIPLICATION SIGN
      MapItem {0xD8u, UnicodeCodepoint(0x0637u)}, // ARABIC LETTER TAH
      MapItem {0xD9u, UnicodeCodepoint(0x0638u)}, // ARABIC LETTER ZAH
      MapItem {0xDAu, UnicodeCodepoint(0x0639u)}, // ARABIC LETTER AIN
      MapItem {0xDBu, UnicodeCodepoint(0x063Au)}, // ARABIC LETTER GHAIN
      MapItem {0xDCu, UnicodeCodepoint(0x0640u)}, // ARABIC TATWEEL
      MapItem {0xDDu, UnicodeCodepoint(0x0641u)}, // ARABIC LETTER FEH
      MapItem {0xDEu, UnicodeCodepoint(0x0642u)}, // ARABIC LETTER QAF
      MapItem {0xDFu, UnicodeCodepoint(0x0643u)}, // ARABIC LETTER KAF
      MapItem {0xE0u, UnicodeCodepoint(0x00E0u)}, // LATIN SMALL LETTER A WITH GRAVE
      MapItem {0xE1u, UnicodeCodepoint(0x0644u)}, // ARABIC LETTER LAM
      MapItem {0xE2u, UnicodeCodepoint(0x00E2u)}, // LATIN SMALL LETTER A WITH CIRCUMFLEX
      MapItem {0xE3u, UnicodeCodepoint(0x0645u)}, // ARABIC LETTER MEEM
      MapItem {0xE4u, UnicodeCodepoint(0x0646u)}, // ARABIC LETTER NOON
      MapItem {0xE5u, UnicodeCodepoint(0x0647u)}, // ARABIC LETTER HEH
      MapItem {0xE6u, UnicodeCodepoint(0x0648u)}, // ARABIC LETTER WAW
      MapItem {0xE7u, UnicodeCodepoint(0x00E7u)}, // LATIN SMALL LETTER C WITH CEDILLA
      MapItem {0xE8u, UnicodeCodepoint(0x00E8u)}, // LATIN SMALL LETTER E WITH GRAVE
      MapItem {0xE9u, UnicodeCodepoint(0x00E9u)}, // LATIN SMALL LETTER E WITH ACUTE
      MapItem {0xEAu, UnicodeCodepoint(0x00EAu)}, // LATIN SMALL LETTER E WITH CIRCUMFLEX
      MapItem {0xEBu, UnicodeCodepoint(0x00EBu)}, // LATIN SMALL LETTER E WITH DIAERESIS
      MapItem {0xECu, UnicodeCodepoint(0x0649u)}, // ARABIC LETTER ALEF MAKSURA
      MapItem {0xEDu, UnicodeCodepoint(0x064Au)}, // ARABIC LETTER YEH
      MapItem {0xEEu, UnicodeCodepoint(0x00EEu)}, // LATIN SMALL LETTER I WITH CIRCUMFLEX
      MapItem {0xEFu, UnicodeCodepoint(0x00EFu)}, // LATIN SMALL LETTER I WITH DIAERESIS
      MapItem {0xF0u, UnicodeCodepoint(0x064Bu)}, // ARABIC FATHATAN
      MapItem {0xF1u, UnicodeCodepoint(0x064Cu)}, // ARABIC DAMMATAN
      MapItem {0xF2u, UnicodeCodepoint(0x064Du)}, // ARABIC KASRATAN
      MapItem {0xF3u, UnicodeCodepoint(0x064Eu)}, // ARABIC FATHA
      MapItem {0xF4u, UnicodeCodepoint(0x00F4u)}, // LATIN SMALL LETTER O WITH CIRCUMFLEX
      MapItem {0xF5u, UnicodeCodepoint(0x064Fu)}, // ARABIC DAMMA
      MapItem {0xF6u, UnicodeCodepoint(0x0650u)}, // ARABIC KASRA
      MapItem {0xF7u, UnicodeCodepoint(0x00F7u)}, // DIVISION SIGN
      MapItem {0xF8u, UnicodeCodepoint(0x0651u)}, // ARABIC SHADDA
      MapItem {0xF9u, UnicodeCodepoint(0x00F9u)}, // LATIN SMALL LETTER U WITH GRAVE
      MapItem {0xFAu, UnicodeCodepoint(0x0652u)}, // ARABIC SUKUN
      MapItem {0xFBu, UnicodeCodepoint(0x00FBu)}, // LATIN SMALL LETTER U WITH CIRCUMFLEX
      MapItem {0xFCu, UnicodeCodepoint(0x00FCu)}, // LATIN SMALL LETTER U WITH DIAERESIS
      MapItem {0xFDu, UnicodeCodepoint(0x200Eu)}, // LEFT-TO-RIGHT MARK
      MapItem {0xFEu, UnicodeCodepoint(0x200Fu)}, // RIGHT-TO-LEFT MARK
      MapItem {0xFFu, UnicodeCodepoint(0x06D2u)}, // ARABIC LETTER YEH BARREE
    };

  public:
    Windows1256Encoding() noexcept : SingleByteEncoding({ Name, MIBenum, WindowsCodePage }, LookupTable(LookupMapping))
    {
    }

    ~Windows1256Encoding() noexcept override = default;
  };
}