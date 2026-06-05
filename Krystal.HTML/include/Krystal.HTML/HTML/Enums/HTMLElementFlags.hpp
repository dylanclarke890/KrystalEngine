#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class HTMLElementFlags : uint8
  {
    None = 0,
    IsMediaElement = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::HTMLElementFlags, 2u);