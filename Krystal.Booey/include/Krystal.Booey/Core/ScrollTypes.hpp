#pragma once

#include "Krystal.Core/Numeric.hpp"

// FIXME: Only the types the computed style uses; the scrolling types come with scrolling.

namespace krys::boo
{
  enum class OverscrollBehavior : uint8
  {
    Auto,
    Contain,
    None
  };

  enum class OverflowAnchor : bool
  {
    Auto,
    None
  };

  enum class ScrollbarWidth : uint8
  {
    Auto,
    Thin,
    None
  };
}
