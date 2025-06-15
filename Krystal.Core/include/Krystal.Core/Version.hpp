#pragma once

#include <optional>

namespace Krys
{
  struct Version
  {
    int Major;
    int Minor;
    std::optional<int> Patch = std::nullopt;

    constexpr Version(int major, int minor) noexcept : Major(major), Minor(minor)
    {
    }

    constexpr Version(int major, int minor, int patch) noexcept : Major(major), Minor(minor), Patch(patch)
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