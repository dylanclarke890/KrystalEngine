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
  constexpr inline const index16_code_point ISO_8859_14_IndexCodePointMap[128] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x1E02},  {34, 0x1E03},
    {35, 0x00A3},  {36, 0x010A},  {37, 0x010B},  {38, 0x1E0A},  {39, 0x00A7},  {40, 0x1E80},  {41, 0x00A9},
    {42, 0x1E82},  {43, 0x1E0B},  {44, 0x1EF2},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x0178},  {48, 0x1E1E},
    {49, 0x1E1F},  {50, 0x0120},  {51, 0x0121},  {52, 0x1E40},  {53, 0x1E41},  {54, 0x00B6},  {55, 0x1E56},
    {56, 0x1E81},  {57, 0x1E57},  {58, 0x1E83},  {59, 0x1E60},  {60, 0x1EF3},  {61, 0x1E84},  {62, 0x1E85},
    {63, 0x1E61},  {64, 0x00C0},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x00C3},  {68, 0x00C4},  {69, 0x00C5},
    {70, 0x00C6},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},  {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},
    {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x0174},  {81, 0x00D1},  {82, 0x00D2},  {83, 0x00D3},
    {84, 0x00D4},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x1E6A},  {88, 0x00D8},  {89, 0x00D9},  {90, 0x00DA},
    {91, 0x00DB},  {92, 0x00DC},  {93, 0x00DD},  {94, 0x0176},  {95, 0x00DF},  {96, 0x00E0},  {97, 0x00E1},
    {98, 0x00E2},  {99, 0x00E3},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x00E6}, {103, 0x00E7}, {104, 0x00E8},
    {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
    {112, 0x0175}, {113, 0x00F1}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x00F5}, {118, 0x00F6},
    {119, 0x1E6B}, {120, 0x00F8}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x00FD},
    {126, 0x0177}, {127, 0x00FF},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    iso_8859_14_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);

    auto first = ::std::ranges::cbegin(ISO_8859_14_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_14_IndexCodePointMap);
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
    iso_8859_14_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = ::std::ranges::cbegin(ISO_8859_14_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_14_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
