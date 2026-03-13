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
  constexpr inline const index16_code_point ISO_8859_16_IndexCodePointMap[128] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0104},  {34, 0x0105},
    {35, 0x0141},  {36, 0x20AC},  {37, 0x201E},  {38, 0x0160},  {39, 0x00A7},  {40, 0x0161},  {41, 0x00A9},
    {42, 0x0218},  {43, 0x00AB},  {44, 0x0179},  {45, 0x00AD},  {46, 0x017A},  {47, 0x017B},  {48, 0x00B0},
    {49, 0x00B1},  {50, 0x010C},  {51, 0x0142},  {52, 0x017D},  {53, 0x201D},  {54, 0x00B6},  {55, 0x00B7},
    {56, 0x017E},  {57, 0x010D},  {58, 0x0219},  {59, 0x00BB},  {60, 0x0152},  {61, 0x0153},  {62, 0x0178},
    {63, 0x017C},  {64, 0x00C0},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x0102},  {68, 0x00C4},  {69, 0x0106},
    {70, 0x00C6},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},  {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},
    {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x0110},  {81, 0x0143},  {82, 0x00D2},  {83, 0x00D3},
    {84, 0x00D4},  {85, 0x0150},  {86, 0x00D6},  {87, 0x015A},  {88, 0x0170},  {89, 0x00D9},  {90, 0x00DA},
    {91, 0x00DB},  {92, 0x00DC},  {93, 0x0118},  {94, 0x021A},  {95, 0x00DF},  {96, 0x00E0},  {97, 0x00E1},
    {98, 0x00E2},  {99, 0x0103},  {100, 0x00E4}, {101, 0x0107}, {102, 0x00E6}, {103, 0x00E7}, {104, 0x00E8},
    {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
    {112, 0x0111}, {113, 0x0144}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x0151}, {118, 0x00F6},
    {119, 0x015B}, {120, 0x0171}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x0119},
    {126, 0x021B}, {127, 0x00FF},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    iso_8859_16_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);
    auto first = ::std::ranges::cbegin(ISO_8859_16_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_16_IndexCodePointMap);
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
    iso_8859_16_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };
    auto first = ::std::ranges::cbegin(ISO_8859_16_IndexCodePointMap);
    auto last = ::std::ranges::cend(ISO_8859_16_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);
    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }
    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
