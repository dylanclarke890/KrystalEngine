#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class AlphaPremultiplication : uint8
  {
    Premultiplied,
    Unpremultiplied
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::AlphaPremultiplication, 2uz);