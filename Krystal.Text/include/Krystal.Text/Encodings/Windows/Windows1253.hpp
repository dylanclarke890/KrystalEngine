#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys
{
  class Windows1253Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"windows-1253";
    static constexpr uint32 MIBenum = 2'253u;
    static constexpr uint32 WindowsCodePage = 1'253u;

    static constexpr Mapping LookupMapping = {
      MapItem {0x80u, UnicodeCodepoint(0x20ACu)}, // EURO SIGN
      MapItem {0x82u, UnicodeCodepoint(0x201Au)}, // SINGLE LOW-9 QUOTATION MARK
      MapItem {0x83u, UnicodeCodepoint(0x0192u)}, // LATIN SMALL LETTER F WITH HOOK
      MapItem {0x84u, UnicodeCodepoint(0x201Eu)}, // DOUBLE LOW-9 QUOTATION MARK
      MapItem {0x85u, UnicodeCodepoint(0x2026u)}, // HORIZONTAL ELLIPSIS
      MapItem {0x86u, UnicodeCodepoint(0x2020u)}, // DAGGER
      MapItem {0x87u, UnicodeCodepoint(0x2021u)}, // DOUBLE DAGGER
      MapItem {0x89u, UnicodeCodepoint(0x2030u)}, // PER MILLE SIGN
      MapItem {0x8Bu, UnicodeCodepoint(0x2039u)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
      MapItem {0x91u, UnicodeCodepoint(0x2018u)}, // LEFT SINGLE QUOTATION MARK
      MapItem {0x92u, UnicodeCodepoint(0x2019u)}, // RIGHT SINGLE QUOTATION MARK
      MapItem {0x93u, UnicodeCodepoint(0x201Cu)}, // LEFT DOUBLE QUOTATION MARK
      MapItem {0x94u, UnicodeCodepoint(0x201Du)}, // RIGHT DOUBLE QUOTATION MARK
      MapItem {0x95u, UnicodeCodepoint(0x2022u)}, // BULLET
      MapItem {0x96u, UnicodeCodepoint(0x2013u)}, // EN DASH
      MapItem {0x97u, UnicodeCodepoint(0x2014u)}, // EM DASH
      MapItem {0x99u, UnicodeCodepoint(0x2122u)}, // TRADE MARK SIGN
      MapItem {0x9Bu, UnicodeCodepoint(0x203Au)}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
      MapItem {0xA0u, UnicodeCodepoint(0x00A0u)}, // NO-BREAK SPACE
      MapItem {0xA1u, UnicodeCodepoint(0x0385u)}, // GREEK DIALYTIKA TONOS
      MapItem {0xA2u, UnicodeCodepoint(0x0386u)}, // GREEK CAPITAL LETTER ALPHA WITH TONOS
      MapItem {0xA3u, UnicodeCodepoint(0x00A3u)}, // POUND SIGN
      MapItem {0xA4u, UnicodeCodepoint(0x00A4u)}, // CURRENCY SIGN
      MapItem {0xA5u, UnicodeCodepoint(0x00A5u)}, // YEN SIGN
      MapItem {0xA6u, UnicodeCodepoint(0x00A6u)}, // BROKEN BAR
      MapItem {0xA7u, UnicodeCodepoint(0x00A7u)}, // SECTION SIGN
      MapItem {0xA8u, UnicodeCodepoint(0x00A8u)}, // DIAERESIS
      MapItem {0xA9u, UnicodeCodepoint(0x00A9u)}, // COPYRIGHT SIGN
      MapItem {0xABu, UnicodeCodepoint(0x00ABu)}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xACu, UnicodeCodepoint(0x00ACu)}, // NOT SIGN
      MapItem {0xADu, UnicodeCodepoint(0x00ADu)}, // SOFT HYPHEN
      MapItem {0xAEu, UnicodeCodepoint(0x00AEu)}, // REGISTERED SIGN
      MapItem {0xAFu, UnicodeCodepoint(0x2015u)}, // HORIZONTAL BAR
      MapItem {0xB0u, UnicodeCodepoint(0x00B0u)}, // DEGREE SIGN
      MapItem {0xB1u, UnicodeCodepoint(0x00B1u)}, // PLUS-MINUS SIGN
      MapItem {0xB2u, UnicodeCodepoint(0x00B2u)}, // SUPERSCRIPT TWO
      MapItem {0xB3u, UnicodeCodepoint(0x00B3u)}, // SUPERSCRIPT THREE
      MapItem {0xB4u, UnicodeCodepoint(0x0384u)}, // GREEK TONOS
      MapItem {0xB5u, UnicodeCodepoint(0x00B5u)}, // MICRO SIGN
      MapItem {0xB6u, UnicodeCodepoint(0x00B6u)}, // PILCROW SIGN
      MapItem {0xB7u, UnicodeCodepoint(0x00B7u)}, // MIDDLE DOT
      MapItem {0xB8u, UnicodeCodepoint(0x0388u)}, // GREEK CAPITAL LETTER EPSILON WITH TONOS
      MapItem {0xB9u, UnicodeCodepoint(0x0389u)}, // GREEK CAPITAL LETTER ETA WITH TONOS
      MapItem {0xBAu, UnicodeCodepoint(0x038Au)}, // GREEK CAPITAL LETTER IOTA WITH TONOS
      MapItem {0xBBu, UnicodeCodepoint(0x00BBu)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
      MapItem {0xBCu, UnicodeCodepoint(0x038Cu)}, // GREEK CAPITAL LETTER OMICRON WITH TONOS
      MapItem {0xBDu, UnicodeCodepoint(0x00BDu)}, // VULGAR FRACTION ONE HALF
      MapItem {0xBEu, UnicodeCodepoint(0x038Eu)}, // GREEK CAPITAL LETTER UPSILON WITH TONOS
      MapItem {0xBFu, UnicodeCodepoint(0x038Fu)}, // GREEK CAPITAL LETTER OMEGA WITH TONOS
      MapItem {0xC0u, UnicodeCodepoint(0x0390u)}, // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
      MapItem {0xC1u, UnicodeCodepoint(0x0391u)}, // GREEK CAPITAL LETTER ALPHA
      MapItem {0xC2u, UnicodeCodepoint(0x0392u)}, // GREEK CAPITAL LETTER BETA
      MapItem {0xC3u, UnicodeCodepoint(0x0393u)}, // GREEK CAPITAL LETTER GAMMA
      MapItem {0xC4u, UnicodeCodepoint(0x0394u)}, // GREEK CAPITAL LETTER DELTA
      MapItem {0xC5u, UnicodeCodepoint(0x0395u)}, // GREEK CAPITAL LETTER EPSILON
      MapItem {0xC6u, UnicodeCodepoint(0x0396u)}, // GREEK CAPITAL LETTER ZETA
      MapItem {0xC7u, UnicodeCodepoint(0x0397u)}, // GREEK CAPITAL LETTER ETA
      MapItem {0xC8u, UnicodeCodepoint(0x0398u)}, // GREEK CAPITAL LETTER THETA
      MapItem {0xC9u, UnicodeCodepoint(0x0399u)}, // GREEK CAPITAL LETTER IOTA
      MapItem {0xCAu, UnicodeCodepoint(0x039Au)}, // GREEK CAPITAL LETTER KAPPA
      MapItem {0xCBu, UnicodeCodepoint(0x039Bu)}, // GREEK CAPITAL LETTER LAMDA
      MapItem {0xCCu, UnicodeCodepoint(0x039Cu)}, // GREEK CAPITAL LETTER MU
      MapItem {0xCDu, UnicodeCodepoint(0x039Du)}, // GREEK CAPITAL LETTER NU
      MapItem {0xCEu, UnicodeCodepoint(0x039Eu)}, // GREEK CAPITAL LETTER XI
      MapItem {0xCFu, UnicodeCodepoint(0x039Fu)}, // GREEK CAPITAL LETTER OMICRON
      MapItem {0xD0u, UnicodeCodepoint(0x03A0u)}, // GREEK CAPITAL LETTER PI
      MapItem {0xD1u, UnicodeCodepoint(0x03A1u)}, // GREEK CAPITAL LETTER RHO
      MapItem {0xD3u, UnicodeCodepoint(0x03A3u)}, // GREEK CAPITAL LETTER SIGMA
      MapItem {0xD4u, UnicodeCodepoint(0x03A4u)}, // GREEK CAPITAL LETTER TAU
      MapItem {0xD5u, UnicodeCodepoint(0x03A5u)}, // GREEK CAPITAL LETTER UPSILON
      MapItem {0xD6u, UnicodeCodepoint(0x03A6u)}, // GREEK CAPITAL LETTER PHI
      MapItem {0xD7u, UnicodeCodepoint(0x03A7u)}, // GREEK CAPITAL LETTER CHI
      MapItem {0xD8u, UnicodeCodepoint(0x03A8u)}, // GREEK CAPITAL LETTER PSI
      MapItem {0xD9u, UnicodeCodepoint(0x03A9u)}, // GREEK CAPITAL LETTER OMEGA
      MapItem {0xDAu, UnicodeCodepoint(0x03AAu)}, // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
      MapItem {0xDBu, UnicodeCodepoint(0x03ABu)}, // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
      MapItem {0xDCu, UnicodeCodepoint(0x03ACu)}, // GREEK SMALL LETTER ALPHA WITH TONOS
      MapItem {0xDDu, UnicodeCodepoint(0x03ADu)}, // GREEK SMALL LETTER EPSILON WITH TONOS
      MapItem {0xDEu, UnicodeCodepoint(0x03AEu)}, // GREEK SMALL LETTER ETA WITH TONOS
      MapItem {0xDFu, UnicodeCodepoint(0x03AFu)}, // GREEK SMALL LETTER IOTA WITH TONOS
      MapItem {0xE0u, UnicodeCodepoint(0x03B0u)}, // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
      MapItem {0xE1u, UnicodeCodepoint(0x03B1u)}, // GREEK SMALL LETTER ALPHA
      MapItem {0xE2u, UnicodeCodepoint(0x03B2u)}, // GREEK SMALL LETTER BETA
      MapItem {0xE3u, UnicodeCodepoint(0x03B3u)}, // GREEK SMALL LETTER GAMMA
      MapItem {0xE4u, UnicodeCodepoint(0x03B4u)}, // GREEK SMALL LETTER DELTA
      MapItem {0xE5u, UnicodeCodepoint(0x03B5u)}, // GREEK SMALL LETTER EPSILON
      MapItem {0xE6u, UnicodeCodepoint(0x03B6u)}, // GREEK SMALL LETTER ZETA
      MapItem {0xE7u, UnicodeCodepoint(0x03B7u)}, // GREEK SMALL LETTER ETA
      MapItem {0xE8u, UnicodeCodepoint(0x03B8u)}, // GREEK SMALL LETTER THETA
      MapItem {0xE9u, UnicodeCodepoint(0x03B9u)}, // GREEK SMALL LETTER IOTA
      MapItem {0xEAu, UnicodeCodepoint(0x03BAu)}, // GREEK SMALL LETTER KAPPA
      MapItem {0xEBu, UnicodeCodepoint(0x03BBu)}, // GREEK SMALL LETTER LAMDA
      MapItem {0xECu, UnicodeCodepoint(0x03BCu)}, // GREEK SMALL LETTER MU
      MapItem {0xEDu, UnicodeCodepoint(0x03BDu)}, // GREEK SMALL LETTER NU
      MapItem {0xEEu, UnicodeCodepoint(0x03BEu)}, // GREEK SMALL LETTER XI
      MapItem {0xEFu, UnicodeCodepoint(0x03BFu)}, // GREEK SMALL LETTER OMICRON
      MapItem {0xF0u, UnicodeCodepoint(0x03C0u)}, // GREEK SMALL LETTER PI
      MapItem {0xF1u, UnicodeCodepoint(0x03C1u)}, // GREEK SMALL LETTER RHO
      MapItem {0xF2u, UnicodeCodepoint(0x03C2u)}, // GREEK SMALL LETTER FINAL SIGMA
      MapItem {0xF3u, UnicodeCodepoint(0x03C3u)}, // GREEK SMALL LETTER SIGMA
      MapItem {0xF4u, UnicodeCodepoint(0x03C4u)}, // GREEK SMALL LETTER TAU
      MapItem {0xF5u, UnicodeCodepoint(0x03C5u)}, // GREEK SMALL LETTER UPSILON
      MapItem {0xF6u, UnicodeCodepoint(0x03C6u)}, // GREEK SMALL LETTER PHI
      MapItem {0xF7u, UnicodeCodepoint(0x03C7u)}, // GREEK SMALL LETTER CHI
      MapItem {0xF8u, UnicodeCodepoint(0x03C8u)}, // GREEK SMALL LETTER PSI
      MapItem {0xF9u, UnicodeCodepoint(0x03C9u)}, // GREEK SMALL LETTER OMEGA
      MapItem {0xFAu, UnicodeCodepoint(0x03CAu)}, // GREEK SMALL LETTER IOTA WITH DIALYTIKA
      MapItem {0xFBu, UnicodeCodepoint(0x03CBu)}, // GREEK SMALL LETTER UPSILON WITH DIALYTIKA
      MapItem {0xFCu, UnicodeCodepoint(0x03CCu)}, // GREEK SMALL LETTER OMICRON WITH TONOS
      MapItem {0xFDu, UnicodeCodepoint(0x03CDu)}, // GREEK SMALL LETTER UPSILON WITH TONOS
      MapItem {0xFEu, UnicodeCodepoint(0x03CEu)}, // GREEK SMALL LETTER OMEGA WITH TONOS
    };

  public:
    Windows1253Encoding() noexcept : SingleByteEncoding({ Name, MIBenum, WindowsCodePage }, LookupTable(LookupMapping))
    {
    }

    ~Windows1253Encoding() noexcept override = default;
  };
}