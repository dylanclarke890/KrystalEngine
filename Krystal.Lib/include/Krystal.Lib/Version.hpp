#pragma once

#include "Krystal.Lib/Types.hpp"
#include <compare>

namespace Krys
{
  struct Version
  {
    uint32 Major;
    uint32 Minor;
    uint32 Patch;

    constexpr Version(uint32 major, uint32 minor) noexcept : Major(major), Minor(minor), Patch(0u)
    {
    }

    constexpr Version(uint32 major, uint32 minor, uint32 patch) noexcept
        : Major(major), Minor(minor), Patch(patch)
    {
    }

    constexpr auto operator<=>(const Version &other) const noexcept
    {
      if (Major != other.Major)
        return Major <=> other.Major;
      if (Minor != other.Minor)
        return Minor <=> other.Minor;
      return Patch <=> other.Patch;
    }
  };
}