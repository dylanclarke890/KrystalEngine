#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::Gfx
{
  enum class VertexInputRate : uint8
  {
    PerVertex,
    PerInstance
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::Gfx::VertexInputRate, 2u);