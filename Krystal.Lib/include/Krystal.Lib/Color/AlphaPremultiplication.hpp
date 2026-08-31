#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys
{
  enum class AlphaPremultiplication : uint8
  {
    Premultiplied,
    Unpremultiplied
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::AlphaPremultiplication, 2uz);