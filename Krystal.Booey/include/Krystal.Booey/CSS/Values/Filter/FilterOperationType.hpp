#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class FilterOperationType : uint8
  {
    None,
    Default,
    Reference, // url(#somefilter)
    Grayscale,
    Sepia,
    Saturate,
    HueRotate,
    Invert,
    AppleInvertLightness,
    Opacity,
    Brightness,
    Contrast,
    Blur,
    DropShadow,
    DropShadowWithStyleColor,
    Passthrough,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::FilterOperationType, 16uz);