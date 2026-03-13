#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <cstddef>
#include <optional>

namespace Krys::Text::EncodingTable
{
#pragma region Table
  constexpr inline const index16_code_point ISO_8859_13_IndexCodePointMap[128] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x201D},  {34, 0x00A2},
    {35, 0x00A3},  {36, 0x00A4},  {37, 0x201E},  {38, 0x00A6},  {39, 0x00A7},  {40, 0x00D8},  {41, 0x00A9},
    {42, 0x0156},  {43, 0x00AB},  {44, 0x00AC},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x00C6},  {48, 0x00B0},
    {49, 0x00B1},  {50, 0x00B2},  {51, 0x00B3},  {52, 0x201C},  {53, 0x00B5},  {54, 0x00B6},  {55, 0x00B7},
    {56, 0x00F8},  {57, 0x00B9},  {58, 0x0157},  {59, 0x00BB},  {60, 0x00BC},  {61, 0x00BD},  {62, 0x00BE},
    {63, 0x00E6},  {64, 0x0104},  {65, 0x012E},  {66, 0x0100},  {67, 0x0106},  {68, 0x00C4},  {69, 0x00C5},
    {70, 0x0118},  {71, 0x0112},  {72, 0x010C},  {73, 0x00C9},  {74, 0x0179},  {75, 0x0116},  {76, 0x0122},
    {77, 0x0136},  {78, 0x012A},  {79, 0x013B},  {80, 0x0160},  {81, 0x0143},  {82, 0x0145},  {83, 0x00D3},
    {84, 0x014C},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x0172},  {89, 0x0141},  {90, 0x015A},
    {91, 0x016A},  {92, 0x00DC},  {93, 0x017B},  {94, 0x017D},  {95, 0x00DF},  {96, 0x0105},  {97, 0x012F},
    {98, 0x0101},  {99, 0x0107},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x0119}, {103, 0x0113}, {104, 0x010D},
    {105, 0x00E9}, {106, 0x017A}, {107, 0x0117}, {108, 0x0123}, {109, 0x0137}, {110, 0x012B}, {111, 0x013C},
    {112, 0x0161}, {113, 0x0144}, {114, 0x0146}, {115, 0x00F3}, {116, 0x014D}, {117, 0x00F5}, {118, 0x00F6},
    {119, 0x00F7}, {120, 0x0173}, {121, 0x0142}, {122, 0x015B}, {123, 0x016B}, {124, 0x00FC}, {125, 0x017C},
    {126, 0x017E}, {127, 0x2019},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    iso_8859_13_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);

    auto first = ::std::ranges::cbegin(ISO_8859_13_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_13_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::lower_bound(first, last, lookupIndex, &LessThanIndex16Target);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    return static_cast<uint_least32_t>(indexAndCodepoint[1]);
  }

  KRYS_NODISCARD constexpr inline std::optional<std::size_t>
    iso_8859_13_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = ::std::ranges::cbegin(ISO_8859_13_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_13_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
