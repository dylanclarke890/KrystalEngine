#pragma once

namespace krys::boo::css
{
  struct CurrentColor
  {
    constexpr bool operator==(const CurrentColor &) const = default;
  };
}