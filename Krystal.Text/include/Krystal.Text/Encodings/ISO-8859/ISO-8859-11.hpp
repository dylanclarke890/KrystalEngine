#pragma once

#include "Krystal.Text/Encodings/SingleByteEncoding.hpp"

namespace Krys::Text
{
  class ISO_8859_11_Encoding : public SingleByteEncoding
  {
    using Mapping = LookupTable::Mapping;
    using MapItem = LookupTable::MapItem;

  public:
    static constexpr utf8_stringview Name = u8"ISO-8859-11";
    static constexpr uint32 MIBenum = 2'259u;
    static constexpr uint32 WindowsCodePage = 28'601u;

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
      MapItem {0xA1u, UnicodeCodepoint(0x0E01u)}, //	THAI CHARACTER KO KAI
      MapItem {0xA2u, UnicodeCodepoint(0x0E02u)}, //	THAI CHARACTER KHO KHAI
      MapItem {0xA3u, UnicodeCodepoint(0x0E03u)}, //	THAI CHARACTER KHO KHUAT
      MapItem {0xA4u, UnicodeCodepoint(0x0E04u)}, //	THAI CHARACTER KHO KHWAI
      MapItem {0xA5u, UnicodeCodepoint(0x0E05u)}, //	THAI CHARACTER KHO KHON
      MapItem {0xA6u, UnicodeCodepoint(0x0E06u)}, //	THAI CHARACTER KHO RAKHANG
      MapItem {0xA7u, UnicodeCodepoint(0x0E07u)}, //	THAI CHARACTER NGO NGU
      MapItem {0xA8u, UnicodeCodepoint(0x0E08u)}, //	THAI CHARACTER CHO CHAN
      MapItem {0xA9u, UnicodeCodepoint(0x0E09u)}, //	THAI CHARACTER CHO CHING
      MapItem {0xAAu, UnicodeCodepoint(0x0E0Au)}, //	THAI CHARACTER CHO CHANG
      MapItem {0xABu, UnicodeCodepoint(0x0E0Bu)}, //	THAI CHARACTER SO SO
      MapItem {0xACu, UnicodeCodepoint(0x0E0Cu)}, //	THAI CHARACTER CHO CHOE
      MapItem {0xADu, UnicodeCodepoint(0x0E0Du)}, //	THAI CHARACTER YO YING
      MapItem {0xAEu, UnicodeCodepoint(0x0E0Eu)}, //	THAI CHARACTER DO CHADA
      MapItem {0xAFu, UnicodeCodepoint(0x0E0Fu)}, //	THAI CHARACTER TO PATAK
      MapItem {0xB0u, UnicodeCodepoint(0x0E10u)}, //	THAI CHARACTER THO THAN
      MapItem {0xB1u, UnicodeCodepoint(0x0E11u)}, //	THAI CHARACTER THO NANGMONTHO
      MapItem {0xB2u, UnicodeCodepoint(0x0E12u)}, //	THAI CHARACTER THO PHUTHAO
      MapItem {0xB3u, UnicodeCodepoint(0x0E13u)}, //	THAI CHARACTER NO NEN
      MapItem {0xB4u, UnicodeCodepoint(0x0E14u)}, //	THAI CHARACTER DO DEK
      MapItem {0xB5u, UnicodeCodepoint(0x0E15u)}, //	THAI CHARACTER TO TAO
      MapItem {0xB6u, UnicodeCodepoint(0x0E16u)}, //	THAI CHARACTER THO THUNG
      MapItem {0xB7u, UnicodeCodepoint(0x0E17u)}, //	THAI CHARACTER THO THAHAN
      MapItem {0xB8u, UnicodeCodepoint(0x0E18u)}, //	THAI CHARACTER THO THONG
      MapItem {0xB9u, UnicodeCodepoint(0x0E19u)}, //	THAI CHARACTER NO NU
      MapItem {0xBAu, UnicodeCodepoint(0x0E1Au)}, //	THAI CHARACTER BO BAIMAI
      MapItem {0xBBu, UnicodeCodepoint(0x0E1Bu)}, //	THAI CHARACTER PO PLA
      MapItem {0xBCu, UnicodeCodepoint(0x0E1Cu)}, //	THAI CHARACTER PHO PHUNG
      MapItem {0xBDu, UnicodeCodepoint(0x0E1Du)}, //	THAI CHARACTER FO FA
      MapItem {0xBEu, UnicodeCodepoint(0x0E1Eu)}, //	THAI CHARACTER PHO PHAN
      MapItem {0xBFu, UnicodeCodepoint(0x0E1Fu)}, //	THAI CHARACTER FO FAN
      MapItem {0xC0u, UnicodeCodepoint(0x0E20u)}, //	THAI CHARACTER PHO SAMPHAO
      MapItem {0xC1u, UnicodeCodepoint(0x0E21u)}, //	THAI CHARACTER MO MA
      MapItem {0xC2u, UnicodeCodepoint(0x0E22u)}, //	THAI CHARACTER YO YAK
      MapItem {0xC3u, UnicodeCodepoint(0x0E23u)}, //	THAI CHARACTER RO RUA
      MapItem {0xC4u, UnicodeCodepoint(0x0E24u)}, //	THAI CHARACTER RU
      MapItem {0xC5u, UnicodeCodepoint(0x0E25u)}, //	THAI CHARACTER LO LING
      MapItem {0xC6u, UnicodeCodepoint(0x0E26u)}, //	THAI CHARACTER LU
      MapItem {0xC7u, UnicodeCodepoint(0x0E27u)}, //	THAI CHARACTER WO WAEN
      MapItem {0xC8u, UnicodeCodepoint(0x0E28u)}, //	THAI CHARACTER SO SALA
      MapItem {0xC9u, UnicodeCodepoint(0x0E29u)}, //	THAI CHARACTER SO RUSI
      MapItem {0xCAu, UnicodeCodepoint(0x0E2Au)}, //	THAI CHARACTER SO SUA
      MapItem {0xCBu, UnicodeCodepoint(0x0E2Bu)}, //	THAI CHARACTER HO HIP
      MapItem {0xCCu, UnicodeCodepoint(0x0E2Cu)}, //	THAI CHARACTER LO CHULA
      MapItem {0xCDu, UnicodeCodepoint(0x0E2Du)}, //	THAI CHARACTER O ANG
      MapItem {0xCEu, UnicodeCodepoint(0x0E2Eu)}, //	THAI CHARACTER HO NOKHUK
      MapItem {0xCFu, UnicodeCodepoint(0x0E2Fu)}, //	THAI CHARACTER PAIYANNOI
      MapItem {0xD0u, UnicodeCodepoint(0x0E30u)}, //	THAI CHARACTER SARA A
      MapItem {0xD1u, UnicodeCodepoint(0x0E31u)}, //	THAI CHARACTER MAI HAN-AKAT
      MapItem {0xD2u, UnicodeCodepoint(0x0E32u)}, //	THAI CHARACTER SARA AA
      MapItem {0xD3u, UnicodeCodepoint(0x0E33u)}, //	THAI CHARACTER SARA AM
      MapItem {0xD4u, UnicodeCodepoint(0x0E34u)}, //	THAI CHARACTER SARA I
      MapItem {0xD5u, UnicodeCodepoint(0x0E35u)}, //	THAI CHARACTER SARA II
      MapItem {0xD6u, UnicodeCodepoint(0x0E36u)}, //	THAI CHARACTER SARA UE
      MapItem {0xD7u, UnicodeCodepoint(0x0E37u)}, //	THAI CHARACTER SARA UEE
      MapItem {0xD8u, UnicodeCodepoint(0x0E38u)}, //	THAI CHARACTER SARA U
      MapItem {0xD9u, UnicodeCodepoint(0x0E39u)}, //	THAI CHARACTER SARA UU
      MapItem {0xDAu, UnicodeCodepoint(0x0E3Au)}, //	THAI CHARACTER PHINTHU
      MapItem {0xDFu, UnicodeCodepoint(0x0E3Fu)}, //	THAI CURRENCY SYMBOL BAHT
      MapItem {0xE0u, UnicodeCodepoint(0x0E40u)}, //	THAI CHARACTER SARA E
      MapItem {0xE1u, UnicodeCodepoint(0x0E41u)}, //	THAI CHARACTER SARA AE
      MapItem {0xE2u, UnicodeCodepoint(0x0E42u)}, //	THAI CHARACTER SARA O
      MapItem {0xE3u, UnicodeCodepoint(0x0E43u)}, //	THAI CHARACTER SARA AI MAIMUAN
      MapItem {0xE4u, UnicodeCodepoint(0x0E44u)}, //	THAI CHARACTER SARA AI MAIMALAI
      MapItem {0xE5u, UnicodeCodepoint(0x0E45u)}, //	THAI CHARACTER LAKKHANGYAO
      MapItem {0xE6u, UnicodeCodepoint(0x0E46u)}, //	THAI CHARACTER MAIYAMOK
      MapItem {0xE7u, UnicodeCodepoint(0x0E47u)}, //	THAI CHARACTER MAITAIKHU
      MapItem {0xE8u, UnicodeCodepoint(0x0E48u)}, //	THAI CHARACTER MAI EK
      MapItem {0xE9u, UnicodeCodepoint(0x0E49u)}, //	THAI CHARACTER MAI THO
      MapItem {0xEAu, UnicodeCodepoint(0x0E4Au)}, //	THAI CHARACTER MAI TRI
      MapItem {0xEBu, UnicodeCodepoint(0x0E4Bu)}, //	THAI CHARACTER MAI CHATTAWA
      MapItem {0xECu, UnicodeCodepoint(0x0E4Cu)}, //	THAI CHARACTER THANTHAKHAT
      MapItem {0xEDu, UnicodeCodepoint(0x0E4Du)}, //	THAI CHARACTER NIKHAHIT
      MapItem {0xEEu, UnicodeCodepoint(0x0E4Eu)}, //	THAI CHARACTER YAMAKKAN
      MapItem {0xEFu, UnicodeCodepoint(0x0E4Fu)}, //	THAI CHARACTER FONGMAN
      MapItem {0xF0u, UnicodeCodepoint(0x0E50u)}, //	THAI DIGIT ZERO
      MapItem {0xF1u, UnicodeCodepoint(0x0E51u)}, //	THAI DIGIT ONE
      MapItem {0xF2u, UnicodeCodepoint(0x0E52u)}, //	THAI DIGIT TWO
      MapItem {0xF3u, UnicodeCodepoint(0x0E53u)}, //	THAI DIGIT THREE
      MapItem {0xF4u, UnicodeCodepoint(0x0E54u)}, //	THAI DIGIT FOUR
      MapItem {0xF5u, UnicodeCodepoint(0x0E55u)}, //	THAI DIGIT FIVE
      MapItem {0xF6u, UnicodeCodepoint(0x0E56u)}, //	THAI DIGIT SIX
      MapItem {0xF7u, UnicodeCodepoint(0x0E57u)}, //	THAI DIGIT SEVEN
      MapItem {0xF8u, UnicodeCodepoint(0x0E58u)}, //	THAI DIGIT EIGHT
      MapItem {0xF9u, UnicodeCodepoint(0x0E59u)}, //	THAI DIGIT NINE
      MapItem {0xFAu, UnicodeCodepoint(0x0E5Au)}, //	THAI CHARACTER ANGKHANKHU
      MapItem {0xFBu, UnicodeCodepoint(0x0E5Bu)}, //	THAI CHARACTER KHOMUT
    };

  public:
    ISO_8859_11_Encoding() noexcept
        : SingleByteEncoding({Name, MIBenum, WindowsCodePage}, LookupTable(LookupMapping))
    {
    }

    ~ISO_8859_11_Encoding() noexcept override = default;
  };
}