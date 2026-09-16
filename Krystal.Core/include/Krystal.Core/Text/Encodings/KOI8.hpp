#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
#pragma region KOI8R

  /// @brief The encoding that matches the KOI-8-R (Russian) Cyrillic encoding specification.
  struct KOI8R : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::KOI8R;
    constexpr static utf8_stringview Name = u8"KOI8-R";
    constexpr static Array<utf8_stringview, 5uz> Aliases = {u8"koi8-r", u8"koi8_r", u8"cskoi8r", u8"koi",
                                                            u8"koi8"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x2500},   {1, 0x2502},   {2, 0x250C},   {3, 0x2510},   {4, 0x2514},   {5, 0x2518},   {6, 0x251C},
      {7, 0x2524},   {8, 0x252C},   {9, 0x2534},   {10, 0x253C},  {11, 0x2580},  {12, 0x2584},  {13, 0x2588},
      {14, 0x258C},  {15, 0x2590},  {16, 0x2591},  {17, 0x2592},  {18, 0x2593},  {19, 0x2320},  {20, 0x25A0},
      {21, 0x2219},  {22, 0x221A},  {23, 0x2248},  {24, 0x2264},  {25, 0x2265},  {26, 0x00A0},  {27, 0x2321},
      {28, 0x00B0},  {29, 0x00B2},  {30, 0x00B7},  {31, 0x00F7},  {32, 0x2550},  {33, 0x2551},  {34, 0x2552},
      {35, 0x0451},  {36, 0x2553},  {37, 0x2554},  {38, 0x2555},  {39, 0x2556},  {40, 0x2557},  {41, 0x2558},
      {42, 0x2559},  {43, 0x255A},  {44, 0x255B},  {45, 0x255C},  {46, 0x255D},  {47, 0x255E},  {48, 0x255F},
      {49, 0x2560},  {50, 0x2561},  {51, 0x0401},  {52, 0x2562},  {53, 0x2563},  {54, 0x2564},  {55, 0x2565},
      {56, 0x2566},  {57, 0x2567},  {58, 0x2568},  {59, 0x2569},  {60, 0x256A},  {61, 0x256B},  {62, 0x256C},
      {63, 0x00A9},  {64, 0x044E},  {65, 0x0430},  {66, 0x0431},  {67, 0x0446},  {68, 0x0434},  {69, 0x0435},
      {70, 0x0444},  {71, 0x0433},  {72, 0x0445},  {73, 0x0438},  {74, 0x0439},  {75, 0x043A},  {76, 0x043B},
      {77, 0x043C},  {78, 0x043D},  {79, 0x043E},  {80, 0x043F},  {81, 0x044F},  {82, 0x0440},  {83, 0x0441},
      {84, 0x0442},  {85, 0x0443},  {86, 0x0436},  {87, 0x0432},  {88, 0x044C},  {89, 0x044B},  {90, 0x0437},
      {91, 0x0448},  {92, 0x044D},  {93, 0x0449},  {94, 0x0447},  {95, 0x044A},  {96, 0x042E},  {97, 0x0410},
      {98, 0x0411},  {99, 0x0426},  {100, 0x0414}, {101, 0x0415}, {102, 0x0424}, {103, 0x0413}, {104, 0x0425},
      {105, 0x0418}, {106, 0x0419}, {107, 0x041A}, {108, 0x041B}, {109, 0x041C}, {110, 0x041D}, {111, 0x041E},
      {112, 0x041F}, {113, 0x042F}, {114, 0x0420}, {115, 0x0421}, {116, 0x0422}, {117, 0x0423}, {118, 0x0416},
      {119, 0x0412}, {120, 0x042C}, {121, 0x042B}, {122, 0x0417}, {123, 0x0428}, {124, 0x042D}, {125, 0x0429},
      {126, 0x0427}, {127, 0x042A},
    }};

    template <CodeUnitInputIterator<KOI8R> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<KOI8R> DecodeOne(TIterator current,
                                                                     TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<KOI8R>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<KOI8R> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<KOI8R>(codePoint);
    }
  };

  static_assert(Encoding<KOI8R>);

#pragma endregion

#pragma region KOI8U

  /// @brief The encoding that matches the KOI-8-U (Ukrainian) Cyrillic encoding specification.
  struct KOI8U : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::KOI8U;
    constexpr static utf8_stringview Name = u8"KOI8-U";
    constexpr static Array<utf8_stringview, 3uz> Aliases = {u8"koi8-u", u8"koi8_u", u8"koi8-ru"};

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x2500},   {1, 0x2502},   {2, 0x250C},   {3, 0x2510},   {4, 0x2514},   {5, 0x2518},   {6, 0x251C},
      {7, 0x2524},   {8, 0x252C},   {9, 0x2534},   {10, 0x253C},  {11, 0x2580},  {12, 0x2584},  {13, 0x2588},
      {14, 0x258C},  {15, 0x2590},  {16, 0x2591},  {17, 0x2592},  {18, 0x2593},  {19, 0x2320},  {20, 0x25A0},
      {21, 0x2219},  {22, 0x221A},  {23, 0x2248},  {24, 0x2264},  {25, 0x2265},  {26, 0x00A0},  {27, 0x2321},
      {28, 0x00B0},  {29, 0x00B2},  {30, 0x00B7},  {31, 0x00F7},  {32, 0x2550},  {33, 0x2551},  {34, 0x2552},
      {35, 0x0451},  {36, 0x0454},  {37, 0x2554},  {38, 0x0456},  {39, 0x0457},  {40, 0x2557},  {41, 0x2558},
      {42, 0x2559},  {43, 0x255A},  {44, 0x255B},  {45, 0x0491},  {46, 0x045E},  {47, 0x255E},  {48, 0x255F},
      {49, 0x2560},  {50, 0x2561},  {51, 0x0401},  {52, 0x0404},  {53, 0x2563},  {54, 0x0406},  {55, 0x0407},
      {56, 0x2566},  {57, 0x2567},  {58, 0x2568},  {59, 0x2569},  {60, 0x256A},  {61, 0x0490},  {62, 0x040E},
      {63, 0x00A9},  {64, 0x044E},  {65, 0x0430},  {66, 0x0431},  {67, 0x0446},  {68, 0x0434},  {69, 0x0435},
      {70, 0x0444},  {71, 0x0433},  {72, 0x0445},  {73, 0x0438},  {74, 0x0439},  {75, 0x043A},  {76, 0x043B},
      {77, 0x043C},  {78, 0x043D},  {79, 0x043E},  {80, 0x043F},  {81, 0x044F},  {82, 0x0440},  {83, 0x0441},
      {84, 0x0442},  {85, 0x0443},  {86, 0x0436},  {87, 0x0432},  {88, 0x044C},  {89, 0x044B},  {90, 0x0437},
      {91, 0x0448},  {92, 0x044D},  {93, 0x0449},  {94, 0x0447},  {95, 0x044A},  {96, 0x042E},  {97, 0x0410},
      {98, 0x0411},  {99, 0x0426},  {100, 0x0414}, {101, 0x0415}, {102, 0x0424}, {103, 0x0413}, {104, 0x0425},
      {105, 0x0418}, {106, 0x0419}, {107, 0x041A}, {108, 0x041B}, {109, 0x041C}, {110, 0x041D}, {111, 0x041E},
      {112, 0x041F}, {113, 0x042F}, {114, 0x0420}, {115, 0x0421}, {116, 0x0422}, {117, 0x0423}, {118, 0x0416},
      {119, 0x0412}, {120, 0x042C}, {121, 0x042B}, {122, 0x0417}, {123, 0x0428}, {124, 0x042D}, {125, 0x0429},
      {126, 0x0427}, {127, 0x042A},
    }};

    template <CodeUnitInputIterator<KOI8U> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<KOI8U> DecodeOne(TIterator current,
                                                                     TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<KOI8U>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<KOI8U> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<KOI8U>(codePoint);
    }
  };

  static_assert(Encoding<KOI8U>);

#pragma endregion
}