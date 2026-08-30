#pragma once

namespace Krys::HTML
{
  struct CurrentColor
  {
    constexpr bool operator==(const CurrentColor &) const = default;
  };
}