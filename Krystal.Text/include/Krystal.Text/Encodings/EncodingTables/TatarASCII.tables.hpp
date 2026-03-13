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

  constexpr inline const index16_code_point TatarASCIIIndexCodePointMap[128] = {
    {0x00, 0x0401}, {0x01, 0x0451}, {0x02, 0x04D8}, {0x03, 0x04E8}, {0x04, 0x04AE}, {0x05, 0x0496},
    {0x06, 0x04A2}, {0x07, 0x04BA}, {0x08, 0x04D9}, {0x09, 0x04E9}, {0x0A, 0x04AF}, {0x0B, 0x0497},
    {0x0C, 0x04A3}, {0x0D, 0x04BB}, {0x0E, 0x041E}, {0x0F, 0x041F}, {0x10, 0x0420}, {17, 0x0421},
    {18, 0x0422},   {19, 0x0423},   {20, 0x0424},   {21, 0x0425},   {22, 0x0426},   {23, 0x0427},
    {24, 0x0428},   {25, 0x0429},   {26, 0x042A},   {27, 0x042B},   {28, 0x042C},   {29, 0x042D},
    {30, 0x042E},   {31, 0x042F},   {32, 0x0430},   {33, 0x0431},   {34, 0x0432},   {35, 0x0433},
    {36, 0x0434},   {37, 0x0435},   {38, 0x0436},   {39, 0x0437},   {40, 0x0438},   {41, 0x0439},
    {42, 0x043A},   {43, 0x043B},   {44, 0x043C},   {45, 0x043D},   {46, 0x043E},   {47, 0x043F},
    {48, 0x2591},   {49, 0x2592},   {50, 0x2593},   {51, 0x2502},   {52, 0x2524},   {53, 0x2561},
    {54, 0x2562},   {55, 0x2556},   {56, 0x2555},   {57, 0x2563},   {58, 0x2551},   {59, 0x2557},
    {60, 0x255D},   {61, 0x255C},   {62, 0x255B},   {63, 0x2510},   {64, 0x2514},   {65, 0x2534},
    {66, 0x252C},   {67, 0x251C},   {68, 0x2500},   {69, 0x253C},   {70, 0x255E},   {71, 0x255F},
    {72, 0x255A},   {73, 0x2554},   {74, 0x2569},   {75, 0x2566},   {76, 0x2560},   {77, 0x2550},
    {78, 0x256C},   {79, 0x2567},   {80, 0x2568},   {81, 0x2564},   {82, 0x2565},   {83, 0x2559},
    {84, 0x2558},   {85, 0x2552},   {86, 0x2553},   {87, 0x256B},   {88, 0x256A},   {89, 0x2518},
    {90, 0x250C},   {91, 0x2588},   {92, 0x2584},   {93, 0x258C},   {94, 0x2590},   {95, 0x2580},
    {96, 0x0440},   {97, 0x0441},   {98, 0x0442},   {99, 0x0443},   {100, 0x0444},  {101, 0x0445},
    {102, 0x0446},  {103, 0x0447},  {104, 0x0448},  {105, 0x0449},  {106, 0x044A},  {107, 0x044B},
    {108, 0x044C},  {109, 0x044D},  {110, 0x044E},  {111, 0x044F},  {112, 0x0401},  {113, 0x0451},
    {114, 0x0404},  {115, 0x0454},  {116, 0x0407},  {117, 0x0457},  {118, 0x040E},  {119, 0x045E},
    {120, 0x00B0},  {121, 0x2219},  {122, 0x00B7},  {123, 0x221A},  {124, 0x2116},  {125, 0x00A4},
    {126, 0x25A0},  {127, 0x00A0},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    tatar_ascii_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index16 lookupIndex = static_cast<index16>(lookupIndexPointer);

    auto first = ::std::ranges::cbegin(TatarASCIIIndexCodePointMap);
    auto last = ::std::ranges::cend(TatarASCIIIndexCodePointMap);
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
    tatar_ascii_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index16_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = ::std::ranges::cbegin(TatarASCIIIndexCodePointMap);
    auto last = ::std::ranges::cend(TatarASCIIIndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index16_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
