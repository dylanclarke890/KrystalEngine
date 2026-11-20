// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
 **********************************************************************
 *   Copyright (C) 1997-2016, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 **********************************************************************
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::text
{
  /// @brief A writing script, by its Unicode script code (ISO 15924 codes in the comments). The values are
  /// ICU's `UScriptCode`s, from which this list is generated; aliases name the same script.
  enum class Script : int16
  {
    Invalid = -1,
    Common = 0,              // Zyyy
    Inherited = 1,           // Zinh
    Arabic = 2,              // Arab
    Armenian = 3,            // Armn
    Bengali = 4,             // Beng
    Bopomofo = 5,            // Bopo
    Cherokee = 6,            // Cher
    Coptic = 7,              // Copt
    Cyrillic = 8,            // Cyrl
    Deseret = 9,             // Dsrt
    Devanagari = 10,         // Deva
    Ethiopic = 11,           // Ethi
    Georgian = 12,           // Geor
    Gothic = 13,             // Goth
    Greek = 14,              // Grek
    Gujarati = 15,           // Gujr
    Gurmukhi = 16,           // Guru
    Han = 17,                // Hani
    Hangul = 18,             // Hang
    Hebrew = 19,             // Hebr
    Hiragana = 20,           // Hira
    Kannada = 21,            // Knda
    Katakana = 22,           // Kana
    Khmer = 23,              // Khmr
    Lao = 24,                // Laoo
    Latin = 25,              // Latn
    Malayalam = 26,          // Mlym
    Mongolian = 27,          // Mong
    Myanmar = 28,            // Mymr
    Ogham = 29,              // Ogam
    OldItalic = 30,          // Ital
    Oriya = 31,              // Orya
    Runic = 32,              // Runr
    Sinhala = 33,            // Sinh
    Syriac = 34,             // Syrc
    Tamil = 35,              // Taml
    Telugu = 36,             // Telu
    Thaana = 37,             // Thaa
    Thai = 38,               // Thai
    Tibetan = 39,            // Tibt
    CanadianAboriginal = 40, // Cans
    Ucas = CanadianAboriginal,
    Yi = 41,                        // Yiii
    Tagalog = 42,                   // Tglg
    Hanunoo = 43,                   // Hano
    Buhid = 44,                     // Buhd
    Tagbanwa = 45,                  // Tagb
    Braille = 46,                   // Brai
    Cypriot = 47,                   // Cprt
    Limbu = 48,                     // Limb
    LinearB = 49,                   // Linb
    Osmanya = 50,                   // Osma
    Shavian = 51,                   // Shaw
    TaiLe = 52,                     // Tale
    Ugaritic = 53,                  // Ugar
    KatakanaOrHiragana = 54,        // Hrkt
    Buginese = 55,                  // Bugi
    Glagolitic = 56,                // Glag
    Kharoshthi = 57,                // Khar
    SylotiNagri = 58,               // Sylo
    NewTaiLue = 59,                 // Talu
    Tifinagh = 60,                  // Tfng
    OldPersian = 61,                // Xpeo
    Balinese = 62,                  // Bali
    Batak = 63,                     // Batk
    Blissymbols = 64,               // Blis
    Brahmi = 65,                    // Brah
    Cham = 66,                      // Cham
    Cirth = 67,                     // Cirt
    OldChurchSlavonicCyrillic = 68, // Cyrs
    DemoticEgyptian = 69,           // Egyd
    HieraticEgyptian = 70,          // Egyh
    EgyptianHieroglyphs = 71,       // Egyp
    Khutsuri = 72,                  // Geok
    SimplifiedHan = 73,             // Hans
    TraditionalHan = 74,            // Hant
    PahawhHmong = 75,               // Hmng
    OldHungarian = 76,              // Hung
    HarappanIndus = 77,             // Inds
    Javanese = 78,                  // Java
    KayahLi = 79,                   // Kali
    LatinFraktur = 80,              // Latf
    LatinGaelic = 81,               // Latg
    Lepcha = 82,                    // Lepc
    LinearA = 83,                   // Lina
    Mandaic = 84,                   // Mand
    Mandaean = Mandaic,
    MayanHieroglyphs = 85,    // Maya
    MeroiticHieroglyphs = 86, // Mero
    Meroitic = MeroiticHieroglyphs,
    Nko = 87,        // Nkoo
    Orkhon = 88,     // Orkh
    OldPermic = 89,  // Perm
    PhagsPa = 90,    // Phag
    Phoenician = 91, // Phnx
    Miao = 92,       // Plrd
    PhoneticPollard = Miao,
    Rongorongo = 93,             // Roro
    Sarati = 94,                 // Sara
    EstrangeloSyriac = 95,       // Syre
    WesternSyriac = 96,          // Syrj
    EasternSyriac = 97,          // Syrn
    Tengwar = 98,                // Teng
    Vai = 99,                    // Vaii
    VisibleSpeech = 100,         // Visp
    Cuneiform = 101,             // Xsux
    UnwrittenLanguages = 102,    // Zxxx
    Unknown = 103,               // Zzzz
    Carian = 104,                // Cari
    Japanese = 105,              // Jpan
    Lanna = 106,                 // Lana
    Lycian = 107,                // Lyci
    Lydian = 108,                // Lydi
    OlChiki = 109,               // Olck
    Rejang = 110,                // Rjng
    Saurashtra = 111,            // Saur
    SignWriting = 112,           // Sgnw
    Sundanese = 113,             // Sund
    Moon = 114,                  // Moon
    MeiteiMayek = 115,           // Mtei
    ImperialAramaic = 116,       // Armi
    Avestan = 117,               // Avst
    Chakma = 118,                // Cakm
    Korean = 119,                // Kore
    Kaithi = 120,                // Kthi
    Manichaean = 121,            // Mani
    InscriptionalPahlavi = 122,  // Phli
    PsalterPahlavi = 123,        // Phlp
    BookPahlavi = 124,           // Phlv
    InscriptionalParthian = 125, // Prti
    Samaritan = 126,             // Samr
    TaiViet = 127,               // Tavt
    MathematicalNotation = 128,  // Zmth
    Symbols = 129,               // Zsym
    Bamum = 130,                 // Bamu
    Lisu = 131,                  // Lisu
    NakhiGeba = 132,             // Nkgb
    OldSouthArabian = 133,       // Sarb
    BassaVah = 134,              // Bass
    Duployan = 135,              // Dupl
    Elbasan = 136,               // Elba
    Grantha = 137,               // Gran
    Kpelle = 138,                // Kpel
    Loma = 139,                  // Loma
    Mende = 140,                 // Mend
    MeroiticCursive = 141,       // Merc
    OldNorthArabian = 142,       // Narb
    Nabataean = 143,             // Nbat
    Palmyrene = 144,             // Palm
    Khudawadi = 145,             // Sind
    Sindhi = Khudawadi,
    WarangCiti = 146,              // Wara
    Afaka = 147,                   // Afak
    Jurchen = 148,                 // Jurc
    Mro = 149,                     // Mroo
    Nushu = 150,                   // Nshu
    Sharada = 151,                 // Shrd
    SoraSompeng = 152,             // Sora
    Takri = 153,                   // Takr
    Tangut = 154,                  // Tang
    Woleai = 155,                  // Wole
    AnatolianHieroglyphs = 156,    // Hluw
    Khojki = 157,                  // Khoj
    Tirhuta = 158,                 // Tirh
    CaucasianAlbanian = 159,       // Aghb
    Mahajani = 160,                // Mahj
    Ahom = 161,                    // Ahom
    Hatran = 162,                  // Hatr
    Modi = 163,                    // Modi
    Multani = 164,                 // Mult
    PauCinHau = 165,               // Pauc
    Siddham = 166,                 // Sidd
    Adlam = 167,                   // Adlm
    Bhaiksuki = 168,               // Bhks
    Marchen = 169,                 // Marc
    Newa = 170,                    // Newa
    Osage = 171,                   // Osge
    HanWithBopomofo = 172,         // Hanb
    Jamo = 173,                    // Jamo
    SymbolsEmoji = 174,            // Zsye
    MasaramGondi = 175,            // Gonm
    Soyombo = 176,                 // Soyo
    ZanabazarSquare = 177,         // Zanb
    Dogra = 178,                   // Dogr
    GunjalaGondi = 179,            // Gong
    Makasar = 180,                 // Maka
    Medefaidrin = 181,             // Medf
    HanifiRohingya = 182,          // Rohg
    Sogdian = 183,                 // Sogd
    OldSogdian = 184,              // Sogo
    Elymaic = 185,                 // Elym
    NyiakengPuachueHmong = 186,    // Hmnp
    Nandinagari = 187,             // Nand
    Wancho = 188,                  // Wcho
    Chorasmian = 189,              // Chrs
    DivesAkuru = 190,              // Diak
    KhitanSmallScript = 191,       // Kits
    Yezidi = 192,                  // Yezi
    CyproMinoan = 193,             // Cpmn
    OldUyghur = 194,               // Ougr
    Tangsa = 195,                  // Tnsa
    Toto = 196,                    // Toto
    Vithkuqi = 197,                // Vith
    Kawi = 198,                    // Kawi
    NagMundari = 199,              // Nagm
    ArabicNastaliq = 200,          // Aran
    Garay = 201,                   // Gara
    GurungKhema = 202,             // Gukh
    KiratRai = 203,                // Krai
    OlOnal = 204,                  // Onao
    Sunuwar = 205,                 // Sunu
    Todhri = 206,                  // Todr
    TuluTigalari = 207,            // Tutg
    BeriaErfe = 208,               // Berf
    Sidetic = 209,                 // Sidt
    TaiYo = 210,                   // Tayo
    TolongSiki = 211,              // Tols
    TraditionalHanWithLatin = 212, // Hntl
  };

  /// @brief The script @p character is written in: its Unicode Script property. Characters shared between
  /// scripts are `Common`, and combining marks `Inherited`.
  KRYS_NODISCARD Script GetScript(char32 character) noexcept;

  /// @brief Whether @p character is used in @p script: its Script or Script_Extensions property includes it.
  KRYS_NODISCARD bool HasScript(char32 character, Script script) noexcept;

  /// @brief @p script's four-letter ISO 15924 code (`Latn`, `Hani`), or empty if it has none.
  KRYS_NODISCARD utf8_stringview ScriptShortName(Script script) noexcept;
}
