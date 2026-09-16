#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::Gfx
{
  enum class VertexInputRate : uint8
  {
    PerVertex,
    PerInstance
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::Gfx::VertexInputRate, 2u);