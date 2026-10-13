#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  // Fill, Stroke, ViewBox are just used for SVG.
  enum class CSSBoxType : uint8
  {
    BoxMissing = 0,
    MarginBox,
    BorderBox,
    PaddingBox,
    ContentBox,
    FillBox,
    StrokeBox,
    ViewBox
  };
}
