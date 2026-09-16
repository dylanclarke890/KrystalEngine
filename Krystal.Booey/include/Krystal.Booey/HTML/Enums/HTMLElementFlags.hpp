#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  enum class HTMLElementFlags : uint8
  {
    None = 0,
    IsMediaElement = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::html::HTMLElementFlags, 2u);