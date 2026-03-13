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
  constexpr inline const index16_code_point ISO_8859_8_IndexCodePointMap[92] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {34, 0x00A2},  {35, 0x00A3},
    {36, 0x00A4},  {37, 0x00A5},  {38, 0x00A6},  {39, 0x00A7},  {40, 0x00A8},  {41, 0x00A9},  {42, 0x00D7},
    {43, 0x00AB},  {44, 0x00AC},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x00AF},  {48, 0x00B0},  {49, 0x00B1},
    {50, 0x00B2},  {51, 0x00B3},  {52, 0x00B4},  {53, 0x00B5},  {54, 0x00B6},  {55, 0x00B7},  {56, 0x00B8},
    {57, 0x00B9},  {58, 0x00F7},  {59, 0x00BB},  {60, 0x00BC},  {61, 0x00BD},  {62, 0x00BE},  {95, 0x2017},
    {96, 0x05D0},  {97, 0x05D1},  {98, 0x05D2},  {99, 0x05D3},  {100, 0x05D4}, {101, 0x05D5}, {102, 0x05D6},
    {103, 0x05D7}, {104, 0x05D8}, {105, 0x05D9}, {106, 0x05DA}, {107, 0x05DB}, {108, 0x05DC}, {109, 0x05DD},
    {110, 0x05DE}, {111, 0x05DF}, {112, 0x05E0}, {113, 0x05E1}, {114, 0x05E2}, {115, 0x05E3}, {116, 0x05E4},
    {117, 0x05E5}, {118, 0x05E6}, {119, 0x05E7}, {120, 0x05E8}, {121, 0x05E9}, {122, 0x05EA}, {125, 0x200E},
    {126, 0x200F},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    iso_8859_8_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);

    auto first = ::std::ranges::cbegin(ISO_8859_8_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_8_IndexCodePointMap);
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
    iso_8859_8_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = ::std::ranges::cbegin(ISO_8859_8_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_8_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
