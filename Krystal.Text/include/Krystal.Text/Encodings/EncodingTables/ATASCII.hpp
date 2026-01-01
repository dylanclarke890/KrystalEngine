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
  constexpr inline const index32_code_point ATASCIIIndexCodePointMap[256] = {
    {0, 0x2665},   {1, 0x251C},    {2, 0x1FB87},  {3, 0x2518},   {4, 0x2524},   {5, 0x2510},   {6, 0x2571},
    {7, 0x2572},   {8, 0x25E2},    {9, 0x2597},   {10, 0x25E3},  {11, 0x259D},  {12, 0x2598},  {13, 0x1FB82},
    {14, 0x2582},  {15, 0x2596},   {16, 0x2663},  {17, 0x250C},  {18, 0x2500},  {19, 0x253C},  {20, 0x2022},
    {21, 0x2584},  {22, 0x258E},   {23, 0x252C},  {24, 0x2534},  {25, 0x258C},  {26, 0x2514},  {27, 0x241B},
    {28, 0x2191},  {29, 0x2193},   {30, 0x2190},  {31, 0x2192},  {32, 0x0020},  {33, 0x0021},  {34, 0x0022},
    {35, 0x0023},  {36, 0x0024},   {37, 0x0025},  {38, 0x0026},  {39, 0x0027},  {40, 0x0028},  {41, 0x0029},
    {42, 0x002A},  {43, 0x002B},   {44, 0x002C},  {45, 0x002D},  {46, 0x002E},  {47, 0x002F},  {48, 0x0030},
    {49, 0x0031},  {50, 0x0032},   {51, 0x0033},  {52, 0x0034},  {53, 0x0035},  {54, 0x0036},  {55, 0x0037},
    {56, 0x0038},  {57, 0x0039},   {58, 0x003A},  {59, 0x003B},  {60, 0x003C},  {61, 0x003D},  {62, 0x003E},
    {63, 0x003F},  {64, 0x0040},   {65, 0x0041},  {66, 0x0042},  {67, 0x0043},  {68, 0x0044},  {69, 0x0045},
    {70, 0x0046},  {71, 0x0047},   {72, 0x0048},  {73, 0x0049},  {74, 0x004A},  {75, 0x004B},  {76, 0x004C},
    {77, 0x004D},  {78, 0x004E},   {79, 0x004F},  {80, 0x0050},  {81, 0x0051},  {82, 0x0052},  {83, 0x0053},
    {84, 0x0054},  {85, 0x0055},   {86, 0x0056},  {87, 0x0057},  {88, 0x0058},  {89, 0x0059},  {90, 0x005A},
    {91, 0x005B},  {92, 0x005C},   {93, 0x005D},  {94, 0x005E},  {95, 0x005F},  {96, 0x2666},  {97, 0x0061},
    {98, 0x0062},  {99, 0x0063},   {100, 0x0064}, {101, 0x0065}, {102, 0x0066}, {103, 0x0067}, {104, 0x0068},
    {105, 0x0069}, {106, 0x006A},  {107, 0x006B}, {108, 0x006C}, {109, 0x006D}, {110, 0x006E}, {111, 0x006F},
    {112, 0x0070}, {113, 0x0071},  {114, 0x0072}, {115, 0x0073}, {116, 0x0074}, {117, 0x0075}, {118, 0x0076},
    {119, 0x0077}, {120, 0x0078},  {121, 0x0079}, {122, 0x007A}, {123, 0x2660}, {124, 0x007C}, {125, 0x1F8B0},
    {126, 0x25C0}, {127, 0x25B6},  {128, 0x2665}, {129, 0x251C}, {130, 0x258A}, {131, 0x2518}, {132, 0x2524},
    {133, 0x2510}, {134, 0x2571},  {135, 0x2572}, {136, 0x25E4}, {137, 0x259B}, {138, 0x25E5}, {139, 0x2599},
    {140, 0x259F}, {141, 0x2586},  {142, 0x2582}, {143, 0x259C}, {144, 0x2663}, {145, 0x250C}, {146, 0x2500},
    {147, 0x253C}, {148, 0x25D8},  {149, 0x2580}, {150, 0x258E}, {151, 0x252C}, {152, 0x2534}, {153, 0x2590},
    {154, 0x2514}, {155, 0x00A0},  {156, 0x2191}, {157, 0x2193}, {158, 0x2190}, {159, 0x2192}, {160, 0x2588},
    {161, 0x0021}, {162, 0x0022},  {163, 0x0023}, {164, 0x0024}, {165, 0x0025}, {166, 0x0026}, {167, 0x0027},
    {168, 0x0028}, {169, 0x0029},  {170, 0x002A}, {171, 0x002B}, {172, 0x002C}, {173, 0x002D}, {174, 0x002E},
    {175, 0x002F}, {176, 0x0030},  {177, 0x0031}, {178, 0x0032}, {179, 0x0033}, {180, 0x0034}, {181, 0x0035},
    {182, 0x0036}, {183, 0x0037},  {184, 0x0038}, {185, 0x0039}, {186, 0x003A}, {187, 0x003B}, {188, 0x003C},
    {189, 0x003D}, {190, 0x003E},  {191, 0x003F}, {192, 0x0040}, {193, 0x0041}, {194, 0x0042}, {195, 0x0043},
    {196, 0x0044}, {197, 0x0045},  {198, 0x0046}, {199, 0x0047}, {200, 0x0048}, {201, 0x0049}, {202, 0x004A},
    {203, 0x004B}, {204, 0x004C},  {205, 0x004D}, {206, 0x004E}, {207, 0x004F}, {208, 0x0050}, {209, 0x0051},
    {210, 0x0052}, {211, 0x0053},  {212, 0x0054}, {213, 0x0055}, {214, 0x0056}, {215, 0x0057}, {216, 0x0058},
    {217, 0x0059}, {218, 0x005A},  {219, 0x005B}, {220, 0x005C}, {221, 0x005D}, {222, 0x005E}, {223, 0x005F},
    {224, 0x2666}, {225, 0x0061},  {226, 0x0062}, {227, 0x0063}, {228, 0x0064}, {229, 0x0065}, {230, 0x0066},
    {231, 0x0067}, {232, 0x0068},  {233, 0x0069}, {234, 0x006A}, {235, 0x006B}, {236, 0x006C}, {237, 0x006D},
    {238, 0x006E}, {239, 0x006F},  {240, 0x0070}, {241, 0x0071}, {242, 0x0072}, {243, 0x0073}, {244, 0x0074},
    {245, 0x0075}, {246, 0x0076},  {247, 0x0077}, {248, 0x0078}, {249, 0x0079}, {250, 0x007A}, {251, 0x2660},
    {252, 0X002D}, {253, 0x1F8B0}, {254, 0x25c0}, {255, 0X25b6},
  };
#pragma endregion

  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    atascii_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    const index32 lookupIndex = static_cast<index32>(lookupIndexPointer);

    auto first = Krys::Ranges::cbegin(ATASCIIIndexCodePointMap);
    auto last = Krys::Ranges::cend(ATASCIIIndexCodePointMap);
    auto itAndLast = Krys::Ranges::lower_bound(first, last, lookupIndex, &LessThanIndex32Target);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index32_code_point &indexAndCodepoint = *itAndLast.Current;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    return static_cast<uint_least32_t>(indexAndCodepoint[1]);
  }

  KRYS_NODISCARD constexpr inline std::optional<std::size_t>
    atascii_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    auto predicate = [&lookupCodePoint](const index32_code_point &value)
    {
      return lookupCodePoint == value[1];
    };

    auto first = Krys::Ranges::cbegin(ATASCIIIndexCodePointMap);
    auto last = Krys::Ranges::cend(ATASCIIIndexCodePointMap);
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }

    const index32_code_point &indexAndCodepoint = *itAndLast.Current;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
