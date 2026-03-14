#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <algorithm>
#include <ranges>

namespace Krys::Text::EncodingTable
{
#pragma region Table
  constexpr inline const index16_code_point ISO_8859_3_IndexCodePointMap[121] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0126},  {34, 0x02D8},
    {35, 0x00A3},  {36, 0x00A4},  {38, 0x0124},  {39, 0x00A7},  {40, 0x00A8},  {41, 0x0130},  {42, 0x015E},
    {43, 0x011E},  {44, 0x0134},  {45, 0x00AD},  {47, 0x017B},  {48, 0x00B0},  {49, 0x0127},  {50, 0x00B2},
    {51, 0x00B3},  {52, 0x00B4},  {53, 0x00B5},  {54, 0x0125},  {55, 0x00B7},  {56, 0x00B8},  {57, 0x0131},
    {58, 0x015F},  {59, 0x011F},  {60, 0x0135},  {61, 0x00BD},  {63, 0x017C},  {64, 0x00C0},  {65, 0x00C1},
    {66, 0x00C2},  {68, 0x00C4},  {69, 0x010A},  {70, 0x0108},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},
    {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},  {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {81, 0x00D1},
    {82, 0x00D2},  {83, 0x00D3},  {84, 0x00D4},  {85, 0x0120},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x011C},
    {89, 0x00D9},  {90, 0x00DA},  {91, 0x00DB},  {92, 0x00DC},  {93, 0x016C},  {94, 0x015C},  {95, 0x00DF},
    {96, 0x00E0},  {97, 0x00E1},  {98, 0x00E2},  {100, 0x00E4}, {101, 0x010B}, {102, 0x0109}, {103, 0x00E7},
    {104, 0x00E8}, {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE},
    {111, 0x00EF}, {113, 0x00F1}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x0121}, {118, 0x00F6},
    {119, 0x00F7}, {120, 0x011D}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x016D},
    {126, 0x015D}, {127, 0x02D9},
  };
#pragma endregion

  KRYS_NODISCARD constexpr Maybe<uint32> ISO_8859_3IndexToCodePoint(size_t lookupIndexPointer) noexcept
  {
    auto lookupIndex = static_cast<const index16>(lookupIndexPointer);
    auto begin = std::ranges::cend(ISO_8859_3_IndexCodePointMap);
    auto end = std::ranges::cend(ISO_8859_3_IndexCodePointMap);
    auto it = std::lower_bound(begin, end, lookupIndex, &LessThanIndex16Target);

    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    return static_cast<uint32>(indexAndCodepoint[1]);
  }

  KRYS_NODISCARD constexpr Maybe<size_t> ISO_8859_3CodePointToIndex(uint32 lookupCodePoint) noexcept
  {
    auto begin = std::ranges::cend(ISO_8859_3_IndexCodePointMap);
    auto end = std::ranges::cend(ISO_8859_3_IndexCodePointMap);
    auto it = std::find_if(begin, end, [&](const auto &value) { return lookupCodePoint == value[1]; });

    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    return static_cast<size_t>(indexAndCodepoint[0]);
  }
}
