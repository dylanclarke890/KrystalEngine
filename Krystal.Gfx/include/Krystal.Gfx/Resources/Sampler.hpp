#pragma once

#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
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

  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };

  struct SamplerDesc
  {
    FilterMode MinFilter {FilterMode::Linear};
    FilterMode MagFilter {FilterMode::Linear};
    WrapMode WrapS {WrapMode::Repeat};
    WrapMode WrapT {WrapMode::Repeat};
    WrapMode WrapR {WrapMode::Repeat};
    float AnisotropicLevel {1.0f};
  };
}