#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::FilterOperationType, 16uz);