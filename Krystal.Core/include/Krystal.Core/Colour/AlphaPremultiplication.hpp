#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys
{
  enum class AlphaPremultiplication : uint8
  {
    Premultiplied,
    Unpremultiplied,
  };

  consteval AlphaPremultiplication MaxEnumValue(AlphaPremultiplication) noexcept
  {
    return AlphaPremultiplication::Unpremultiplied;
  }
}
