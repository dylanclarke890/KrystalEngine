#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#canvasfontstretch
  enum class CanvasFontStretch : uint8
  {
    UltraCondensed,
    ExtraCondensed,
    Condensed,
    SemiCondensed,
    Normal,
    SemiExpanded,
    Expanded,
    ExtraExpanded,
    UltraExpanded
  };
}
