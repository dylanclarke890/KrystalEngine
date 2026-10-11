#pragma once

namespace krys::boo::css
{
  struct CurrentColor
  {
    KRYS_NODISCARD constexpr bool operator==(const CurrentColor &) const noexcept = default;
  };
}