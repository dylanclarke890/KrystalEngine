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
  constexpr inline const index16_code_point ISO_8859_6_IndexCodePointMap[83] = {
    {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
    {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
    {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
    {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
    {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {36, 0x00A4},  {44, 0x060C},
    {45, 0x00AD},  {59, 0x061B},  {63, 0x061F},  {65, 0x0621},  {66, 0x0622},  {67, 0x0623},  {68, 0x0624},
    {69, 0x0625},  {70, 0x0626},  {71, 0x0627},  {72, 0x0628},  {73, 0x0629},  {74, 0x062A},  {75, 0x062B},
    {76, 0x062C},  {77, 0x062D},  {78, 0x062E},  {79, 0x062F},  {80, 0x0630},  {81, 0x0631},  {82, 0x0632},
    {83, 0x0633},  {84, 0x0634},  {85, 0x0635},  {86, 0x0636},  {87, 0x0637},  {88, 0x0638},  {89, 0x0639},
    {90, 0x063A},  {96, 0x0640},  {97, 0x0641},  {98, 0x0642},  {99, 0x0643},  {100, 0x0644}, {101, 0x0645},
    {102, 0x0646}, {103, 0x0647}, {104, 0x0648}, {105, 0x0649}, {106, 0x064A}, {107, 0x064B}, {108, 0x064C},
    {109, 0x064D}, {110, 0x064E}, {111, 0x064F}, {112, 0x0650}, {113, 0x0651}, {114, 0x0652},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    iso_8859_6_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);

    auto first = Krys::Ranges::cbegin(ISO_8859_6_IndexCodePointMap);
    auto last = Krys::Ranges::cend(ISO_8859_6_IndexCodePointMap);
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
    iso_8859_6_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = Krys::Ranges::cbegin(ISO_8859_6_IndexCodePointMap);
    auto last = Krys::Ranges::cend(ISO_8859_6_IndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
