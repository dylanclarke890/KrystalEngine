#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ColorType : uint8
  {
    None = 0,
    Absolute = 1 << 0,
    Current = 1 << 1,
    System = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::ColorType, 3uz)
