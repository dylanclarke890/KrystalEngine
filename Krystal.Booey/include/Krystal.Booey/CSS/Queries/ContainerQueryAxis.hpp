#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css::cq
{
  enum class Axis : uint8
  {
    None = 0,
    Block = 1 << 0,
    Inline = 1 << 1,
    Width = 1 << 2,
    Height = 1 << 3,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::cq::Axis, 4uz);
