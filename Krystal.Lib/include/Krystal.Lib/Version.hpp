#pragma once

namespace Krys
{
  struct Version
  {
    unsigned int Major;
    unsigned int Minor;
    unsigned int Patch;

    constexpr Version(unsigned int major, unsigned int minor) noexcept : Major(major), Minor(minor), Patch(0)
    {
    }

    constexpr Version(unsigned int major, unsigned int minor, unsigned int patch) noexcept
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