#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  // https://drafts.csswg.org/css-anchor-position-1/#typedef-anchor-size
  enum class AnchorSizeDimension : uint8
  {
    Width,
    Height,
    Block,
    Inline,
    SelfBlock,
    SelfInline
  };
}
