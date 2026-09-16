#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
#pragma region MacCyrillic

  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  struct MacCyrillic : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::MacCyrillic;
    constexpr static utf8_stringview Name = u8"x-mac-cyrillic";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"x-mac-cyrillic", u8"x-mac-ukranian"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0410},   {1, 0x0411},   {2, 0x0412},   {3, 0x0413},   {4, 0x0414},   {5, 0x0415},   {6, 0x0416},
      {7, 0x0417},   {8, 0x0418},   {9, 0x0419},   {10, 0x041A},  {11, 0x041B},  {12, 0x041C},  {13, 0x041D},
      {14, 0x041E},  {15, 0x041F},  {16, 0x0420},  {17, 0x0421},  {18, 0x0422},  {19, 0x0423},  {20, 0x0424},
      {21, 0x0425},  {22, 0x0426},  {23, 0x0427},  {24, 0x0428},  {25, 0x0429},  {26, 0x042A},  {27, 0x042B},
      {28, 0x042C},  {29, 0x042D},  {30, 0x042E},  {31, 0x042F},  {32, 0x2020},  {33, 0x00B0},  {34, 0x0490},
      {35, 0x00A3},  {36, 0x00A7},  {37, 0x2022},  {38, 0x00B6},  {39, 0x0406},  {40, 0x00AE},  {41, 0x00A9},
      {42, 0x2122},  {43, 0x0402},  {44, 0x0452},  {45, 0x2260},  {46, 0x0403},  {47, 0x0453},  {48, 0x221E},
      {49, 0x00B1},  {50, 0x2264},  {51, 0x2265},  {52, 0x0456},  {53, 0x00B5},  {54, 0x0491},  {55, 0x0408},
      {56, 0x0404},  {57, 0x0454},  {58, 0x0407},  {59, 0x0457},  {60, 0x0409},  {61, 0x0459},  {62, 0x040A},
      {63, 0x045A},  {64, 0x0458},  {65, 0x0405},  {66, 0x00AC},  {67, 0x221A},  {68, 0x0192},  {69, 0x2248},
      {70, 0x2206},  {71, 0x00AB},  {72, 0x00BB},  {73, 0x2026},  {74, 0x00A0},  {75, 0x040B},  {76, 0x045B},
      {77, 0x040C},  {78, 0x045C},  {79, 0x0455},  {80, 0x2013},  {81, 0x2014},  {82, 0x201C},  {83, 0x201D},
      {84, 0x2018},  {85, 0x2019},  {86, 0x00F7},  {87, 0x201E},  {88, 0x040E},  {89, 0x045E},  {90, 0x040F},
      {91, 0x045F},  {92, 0x2116},  {93, 0x0401},  {94, 0x0451},  {95, 0x044F},  {96, 0x0430},  {97, 0x0431},
      {98, 0x0432},  {99, 0x0433},  {100, 0x0434}, {101, 0x0435}, {102, 0x0436}, {103, 0x0437}, {104, 0x0438},
      {105, 0x0439}, {106, 0x043A}, {107, 0x043B}, {108, 0x043C}, {109, 0x043D}, {110, 0x043E}, {111, 0x043F},
      {112, 0x0440}, {113, 0x0441}, {114, 0x0442}, {115, 0x0443}, {116, 0x0444}, {117, 0x0445}, {118, 0x0446},
      {119, 0x0447}, {120, 0x0448}, {121, 0x0449}, {122, 0x044A}, {123, 0x044B}, {124, 0x044C}, {125, 0x044D},
      {126, 0x044E}, {127, 0x20AC},
    }};

    template <CodeUnitInputIterator<MacCyrillic> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<MacCyrillic> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<MacCyrillic>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<MacCyrillic> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<MacCyrillic>(codePoint);
    }
  };

  static_assert(Encoding<MacCyrillic>);

#pragma endregion

#pragma region MacRoman

  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  struct MacRoman : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::MacRoman;
    constexpr static utf8_stringview Name = u8"macintosh";
    constexpr static Array<utf8_stringview, 4uz> Aliases = {u8"macintosh", u8"csmacintosh", u8"mac",
                                                            u8"x-mac-roman"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x00C4},   {1, 0x00C5},   {2, 0x00C7},   {3, 0x00C9},   {4, 0x00D1},   {5, 0x00D6},   {6, 0x00DC},
      {7, 0x00E1},   {8, 0x00E0},   {9, 0x00E2},   {10, 0x00E4},  {11, 0x00E3},  {12, 0x00E5},  {13, 0x00E7},
      {14, 0x00E9},  {15, 0x00E8},  {16, 0x00EA},  {17, 0x00EB},  {18, 0x00ED},  {19, 0x00EC},  {20, 0x00EE},
      {21, 0x00EF},  {22, 0x00F1},  {23, 0x00F3},  {24, 0x00F2},  {25, 0x00F4},  {26, 0x00F6},  {27, 0x00F5},
      {28, 0x00FA},  {29, 0x00F9},  {30, 0x00FB},  {31, 0x00FC},  {32, 0x2020},  {33, 0x00B0},  {34, 0x00A2},
      {35, 0x00A3},  {36, 0x00A7},  {37, 0x2022},  {38, 0x00B6},  {39, 0x00DF},  {40, 0x00AE},  {41, 0x00A9},
      {42, 0x2122},  {43, 0x00B4},  {44, 0x00A8},  {45, 0x2260},  {46, 0x00C6},  {47, 0x00D8},  {48, 0x221E},
      {49, 0x00B1},  {50, 0x2264},  {51, 0x2265},  {52, 0x00A5},  {53, 0x00B5},  {54, 0x2202},  {55, 0x2211},
      {56, 0x220F},  {57, 0x03C0},  {58, 0x222B},  {59, 0x00AA},  {60, 0x00BA},  {61, 0x03A9},  {62, 0x00E6},
      {63, 0x00F8},  {64, 0x00BF},  {65, 0x00A1},  {66, 0x00AC},  {67, 0x221A},  {68, 0x0192},  {69, 0x2248},
      {70, 0x2206},  {71, 0x00AB},  {72, 0x00BB},  {73, 0x2026},  {74, 0x00A0},  {75, 0x00C0},  {76, 0x00C3},
      {77, 0x00D5},  {78, 0x0152},  {79, 0x0153},  {80, 0x2013},  {81, 0x2014},  {82, 0x201C},  {83, 0x201D},
      {84, 0x2018},  {85, 0x2019},  {86, 0x00F7},  {87, 0x25CA},  {88, 0x00FF},  {89, 0x0178},  {90, 0x2044},
      {91, 0x20AC},  {92, 0x2039},  {93, 0x203A},  {94, 0xFB01},  {95, 0xFB02},  {96, 0x2021},  {97, 0x00B7},
      {98, 0x201A},  {99, 0x201E},  {100, 0x2030}, {101, 0x00C2}, {102, 0x00CA}, {103, 0x00C1}, {104, 0x00CB},
      {105, 0x00C8}, {106, 0x00CD}, {107, 0x00CE}, {108, 0x00CF}, {109, 0x00CC}, {110, 0x00D3}, {111, 0x00D4},
      {112, 0xF8FF}, {113, 0x00D2}, {114, 0x00DA}, {115, 0x00DB}, {116, 0x00D9}, {117, 0x0131}, {118, 0x02C6},
      {119, 0x02DC}, {120, 0x00AF}, {121, 0x02D8}, {122, 0x02D9}, {123, 0x02DA}, {124, 0x00B8}, {125, 0x02DD},
      {126, 0x02DB}, {127, 0x02C7},
    }};

    template <CodeUnitInputIterator<MacRoman> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<MacRoman> DecodeOne(TIterator current,
                                                                        TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<MacRoman>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<MacRoman> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<MacRoman>(codePoint);
    }
  };

  static_assert(Encoding<MacRoman>);

#pragma endregion
}