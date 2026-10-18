#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys
{
  enum class AlphaPremultiplication : uint8
  {
    Premultiplied,
    Unpremultiplied
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::AlphaPremultiplication, 2uz);