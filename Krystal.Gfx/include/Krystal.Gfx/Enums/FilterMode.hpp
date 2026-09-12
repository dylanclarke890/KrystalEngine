#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::Gfx
{
  enum class FilterMode : uint8
  {
    Nearest = 0,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
  };
}