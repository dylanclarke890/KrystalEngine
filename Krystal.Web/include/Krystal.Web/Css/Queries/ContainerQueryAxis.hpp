#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web::css::cq
{
  enum class Axis : uint8
  {
    Block,
    Inline,
    Width,
    Height,
  };

  consteval Axis MaxEnumValue(Axis) noexcept
  {
    return Axis::Height;
  }
}
