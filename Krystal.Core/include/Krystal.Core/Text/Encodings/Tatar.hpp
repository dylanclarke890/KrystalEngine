#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
#pragma region TatarANSI

  /// @brief The encoding that matches the TATAR encoding (ANSI variant, Based on Windows Code Page 1252.)
  struct TatarANSI : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::TatarANSI;
    constexpr static utf8_stringview Name = u8"TATAR-ANSI";
    constexpr static Array<utf8_stringview, 1uz> Aliases = {u8"tatar-ansi"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x04D8},   {1, 0x0403},   {2, 0x201A},   {3, 0x0453},   {4, 0x201E},   {5, 0x2026},   {6, 0x2020},
      {7, 0x2021},   {8, 0x20AC},   {9, 0x2030},   {10, 0x04E8},  {11, 0x2039},  {12, 0x04AE},  {13, 0x0496},
      {14, 0x04A2},  {15, 0x04BA},  {16, 0x04D9},  {17, 0x2018},  {18, 0x2019},  {19, 0x201C},  {20, 0x201D},
      {21, 0x2022},  {22, 0x2013},  {23, 0x2014},  {24, 0xFFFF},  {25, 0x2122},  {26, 0x04E9},  {27, 0x203A},
      {28, 0x04AF},  {29, 0x0497},  {30, 0x04A3},  {31, 0x04BB},  {32, 0x00A0},  {33, 0x040E},  {34, 0x045E},
      {35, 0x0408},  {36, 0x00A4},  {37, 0x0490},  {38, 0x00A6},  {39, 0x00A7},  {40, 0x0401},  {41, 0x00A9},
      {42, 0x0404},  {43, 0x00AB},  {44, 0x00AC},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x0407},  {48, 0x00B0},
      {49, 0x00B1},  {50, 0x0406},  {51, 0x0456},  {52, 0x0491},  {53, 0x00B5},  {54, 0x00B6},  {55, 0x00B7},
      {56, 0x0451},  {57, 0x2116},  {58, 0x0454},  {59, 0x00BB},  {60, 0x0458},  {61, 0x0405},  {62, 0x0455},
      {63, 0x0457},  {64, 0x0410},  {65, 0x0411},  {66, 0x0412},  {67, 0x0413},  {68, 0x0414},  {69, 0x0415},
      {70, 0x0416},  {71, 0x0417},  {72, 0x0418},  {73, 0x0419},  {74, 0x041A},  {75, 0x041B},  {76, 0x041C},
      {77, 0x041D},  {78, 0x041E},  {79, 0x041F},  {80, 0x0420},  {81, 0x0421},  {82, 0x0422},  {83, 0x0423},
      {84, 0x0424},  {85, 0x0425},  {86, 0x0426},  {87, 0x0427},  {88, 0x0428},  {89, 0x0429},  {90, 0x042A},
      {91, 0x042B},  {92, 0x042C},  {93, 0x042D},  {94, 0x042E},  {95, 0x042F},  {96, 0x0430},  {97, 0x0431},
      {98, 0x0432},  {99, 0x0433},  {100, 0x0434}, {101, 0x0435}, {102, 0x0436}, {103, 0x0437}, {104, 0x0438},
      {105, 0x0439}, {106, 0x043A}, {107, 0x043B}, {108, 0x043C}, {109, 0x043D}, {110, 0x043E}, {111, 0x043F},
      {112, 0x0440}, {113, 0x0441}, {114, 0x0442}, {115, 0x0443}, {116, 0x0444}, {117, 0x0445}, {118, 0x0446},
      {119, 0x0447}, {120, 0x0448}, {121, 0x0449}, {122, 0x044A}, {123, 0x044B}, {124, 0x044C}, {125, 0x044D},
      {126, 0x044E}, {127, 0x044F},
    }};

    template <CodeUnitInputIterator<TatarANSI> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<TatarANSI> DecodeOne(TIterator current,
                                                                         TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<TatarANSI>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<TatarANSI> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<TatarANSI>(codePoint);
    }
  };

  static_assert(Encoding<TatarANSI>);

#pragma endregion

#pragma region TatarASCII

  /// @brief The encoding that matches the TATAR encoding specification ("ASCII" variant, IBM Code Page 866).
  struct TatarASCII : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::TatarASCII;
    constexpr static utf8_stringview Name = u8"TATAR-ASCII";
    constexpr static Array<utf8_stringview, 1uz> Aliases = {u8"tatar-ascii"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0401},   {1, 0x0451},   {2, 0x04D8},   {3, 0x04E8},   {4, 0x04AE},   {5, 0x0496},   {6, 0x04A2},
      {7, 0x04BA},   {8, 0x04D9},   {9, 0x04E9},   {10, 0x04AF},  {11, 0x0497},  {12, 0x04A3},  {13, 0x04BB},
      {14, 0x041E},  {15, 0x041F},  {16, 0x0420},  {17, 0x0421},  {18, 0x0422},  {19, 0x0423},  {20, 0x0424},
      {21, 0x0425},  {22, 0x0426},  {23, 0x0427},  {24, 0x0428},  {25, 0x0429},  {26, 0x042A},  {27, 0x042B},
      {28, 0x042C},  {29, 0x042D},  {30, 0x042E},  {31, 0x042F},  {32, 0x0430},  {33, 0x0431},  {34, 0x0432},
      {35, 0x0433},  {36, 0x0434},  {37, 0x0435},  {38, 0x0436},  {39, 0x0437},  {40, 0x0438},  {41, 0x0439},
      {42, 0x043A},  {43, 0x043B},  {44, 0x043C},  {45, 0x043D},  {46, 0x043E},  {47, 0x043F},  {48, 0x2591},
      {49, 0x2592},  {50, 0x2593},  {51, 0x2502},  {52, 0x2524},  {53, 0x2561},  {54, 0x2562},  {55, 0x2556},
      {56, 0x2555},  {57, 0x2563},  {58, 0x2551},  {59, 0x2557},  {60, 0x255D},  {61, 0x255C},  {62, 0x255B},
      {63, 0x2510},  {64, 0x2514},  {65, 0x2534},  {66, 0x252C},  {67, 0x251C},  {68, 0x2500},  {69, 0x253C},
      {70, 0x255E},  {71, 0x255F},  {72, 0x255A},  {73, 0x2554},  {74, 0x2569},  {75, 0x2566},  {76, 0x2560},
      {77, 0x2550},  {78, 0x256C},  {79, 0x2567},  {80, 0x2568},  {81, 0x2564},  {82, 0x2565},  {83, 0x2559},
      {84, 0x2558},  {85, 0x2552},  {86, 0x2553},  {87, 0x256B},  {88, 0x256A},  {89, 0x2518},  {90, 0x250C},
      {91, 0x2588},  {92, 0x2584},  {93, 0x258C},  {94, 0x2590},  {95, 0x2580},  {96, 0x0440},  {97, 0x0441},
      {98, 0x0442},  {99, 0x0443},  {100, 0x0444}, {101, 0x0445}, {102, 0x0446}, {103, 0x0447}, {104, 0x0448},
      {105, 0x0449}, {106, 0x044A}, {107, 0x044B}, {108, 0x044C}, {109, 0x044D}, {110, 0x044E}, {111, 0x044F},
      {112, 0x0401}, {113, 0x0451}, {114, 0x0404}, {115, 0x0454}, {116, 0x0407}, {117, 0x0457}, {118, 0x040E},
      {119, 0x045E}, {120, 0x00B0}, {121, 0x2219}, {122, 0x00B7}, {123, 0x221A}, {124, 0x2116}, {125, 0x00A4},
      {126, 0x25A0}, {127, 0x00A0},
    }};

    template <CodeUnitInputIterator<TatarASCII> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<TatarASCII> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<TatarASCII>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<TatarASCII> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<TatarASCII>(codePoint);
    }
  };

  static_assert(Encoding<TatarASCII>);

#pragma endregion
}