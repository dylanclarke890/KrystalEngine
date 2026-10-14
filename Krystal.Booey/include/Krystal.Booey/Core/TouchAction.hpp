#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class TouchAction : uint8
  {
    Auto,
    None,
    Manipulation,
    PanX,
    PanY,
    PinchZoom,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TouchAction, 6uz);
